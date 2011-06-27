/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#include "BaseService.h"
#include "Parameters.h"
#include "CustomCnQThread.h"
#include "FansThread.h"
#include "Registry.h"
#include "StringUtils.h"
#include "TurboManager.h"

#pragma comment(lib, "../WinRing0/WinRing0.lib")
#pragma comment(lib, "../WinRing0/WinRing0x64.lib")



/*
 * To create your own service, customize the macros below, derive
 * from BaseService and set ServiceType to your service type.
 * Service.cpp should then include your header file instead of this
 * one.
 */

#define NAME "BrazosTweaker"
#define DISPLAY_NAME "BrazosTweaker service"
#define DESCRIPTION "Applies custom P-state settings and controls custom Cool & Quiet."
#define START_TYPE SERVICE_AUTO_START // SERVICE_DEMAND_START, SERVICE_DISABLED
//#define ENABLE_DELAYED_AUTOSTART

class BrazosTweakerService;
typedef BrazosTweakerService ServiceType;

class BrazosTweakerService : public BaseService
{
public:

	BrazosTweakerService(SERVICE_STATUS_HANDLE statusHandle, const SERVICE_STATUS& status) :
		BaseService(statusHandle, status),
		_key(NULL),
		_customCnQThread(NULL),
		_fansThread(NULL)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);

		_numCores = info.dwNumberOfProcessors;
	}

	virtual ~BrazosTweakerService()
	{
		if (_key != NULL)
			RegCloseKey(NULL);
	}


	bool OnInitialize()
	{
		if (!ReadFromRegistry())
			return false;

		if (!(_params.EnableCustomPStates || _params.EnableCustomCnQ || _params.EnableFanManagement) ||
		    !InitializeOls())
		{
			return false;
		}

		if (GetDllStatus() != 0)
		{
			DeinitializeOls();
			return false;
		}

		return true;
	}

	void OnRun()
	{
		// switch to a high process priority
		const HANDLE currentProcess = GetCurrentProcess();
		SetPriorityClass(currentProcess, HIGH_PRIORITY_CLASS);

		Lock();
		{
			if (!StartWork())
				Stop();
		}
		Unlock();
	}

	void OnStop()
	{
		// stop the other threads, let them terminate and then delete them
		delete _customCnQThread; _customCnQThread = NULL;
		delete _fansThread; _fansThread = NULL;

		DeinitializeOls();
	}

	void OnPowerEvent(DWORD eventType, PPOWERBROADCAST_SETTING setting)
	{
		if (eventType == PBT_APMRESUMEAUTOMATIC)
		{
			// the system just resumed from standby/hibernation
			// while the software (RAM) state is the same as before standby,
			// the hardware state isn't (CPU MSRs and fan controller registers)
			// => desync, start over
			Lock();
			{
				// stop the other threads, let them terminate and then delete them
				delete _customCnQThread; _customCnQThread = NULL;
				delete _fansThread; _fansThread = NULL;

				StartWork();
			}
			Unlock();
		}
	}


private:

	int _numCores;

	HKEY _key;
	Parameters _params;

	CustomCnQThread* _customCnQThread;
	FansThread* _fansThread;


	/// <summary>Starts the work and returns true if there actually is work to do.</summary>
	bool StartWork()
	{
		bool result = false;

		if (_params.EnableCustomPStates)
		{
			ApplyCustomPStates();
			UnlockPStates();

			// the service needs to be kept alive to re-apply custom P-states when
			// the system resumes from standby
			result = true;
		}

		if (_params.TurboCores >= 0)
		{
			if (_params.TurboCores == 0)
				TurboManager::Set(false);
			else
			{
				DWORD eax, ebx, ecx, edx;
				if (Cpuid(0x80000008u, &eax, &ebx, &ecx, &edx))
				{
					const int numCores = (ecx & 0xFF) + 1;
					const int numIdleCores = std::max(1, numCores - _params.TurboCores);
					TurboManager::SetNumIdleCores(numIdleCores);
				}

				TurboManager::Set(true);
			}

			result = true;
		}

		if (_params.EnableCustomCnQ)
		{
			_customCnQThread = new CustomCnQThread(_params, _key);
			_customCnQThread->Start();

			result = true;
		}

		if (_params.EnableFanManagement)
		{
			try
			{
				_fansThread = new FansThread(_params);
				_fansThread->Start();

				result = true;
			}
			catch (std::exception)
			{
			}
		}

		return result;
	}


	/// <summary>
	/// Saves custom P-state settings to the cores' MSRs.
	/// If a core is in a P-state being customized, the settings are applied immediately.
	/// </summary>
	void ApplyCustomPStates()
	{
		bool customized[5];
		for (int i = 0; i < 5; i++)
			customized[i] = !_params.Msrs[i].empty();

		// try to temporarily set the number of boosted (Turbo) P-states to 0
		// this should suspend the restriction of software P-state multis by F3x1F0[MaxSwPstateCpuCof]
		const bool turboEnabled = TurboManager::IsEnabled();
		int boostedStates = TurboManager::GetNumBoostedStates(); // the number of boosted states may be 1 even if the Turbo is disabled
		if (boostedStates != 0)
		{
			TurboManager::Set(false);
			boostedStates = TurboManager::GetNumBoostedStates();
		}

		// get the max enabled software P-state from core 0
		DWORD lower, higher;
		RdmsrTx(0xC0010061u, &lower, &higher, 1);
		const int maxEnabledSwPState = (lower >> 4) & 7;

		// switch temporarily to a time-critical (absolutely highest) thread priority
		// (we try not to interfere with default C&Q here)
		HANDLE currentThread = GetCurrentThread();
		const int previousPriority = GetThreadPriority(currentThread);
		SetThreadPriority(currentThread, THREAD_PRIORITY_TIME_CRITICAL);

		// perform one iteration in each core
		int* currentHwPStates = new int[_numCores];
		for (int i = 0; i < _numCores; i++)
		{
			const DWORD_PTR affinityMask = (DWORD_PTR)1 << i;

			// get the current core's current hardware P-state
			int& currentHwPState = currentHwPStates[i];
			RdmsrTx(0xC0010071u, &lower, &higher, affinityMask);
			currentHwPState = (lower >> 16) & 7;

			// customize all hardware P-states except the currently active one
			for (int state = 0; state < 5; state++)
			{
				if (customized[state] && state != currentHwPState)
					SavePState(state, _params.Msrs[state][i], i);
			}

			if (customized[currentHwPState])
			{
				// switch temporarily to another (possibly customized) software P-state
				// default: to software P0, which should specify a higher multi and a higher voltage than
				//   the current state (voltage applied before multi when switching down)
				// if software P0 or boost is currently active: try the highest enabled software P-state
				//   it should specify a lower multi and a lower voltage than the current state
				//   (multi applied before voltage when switching up)
				const int tempSwPState = (currentHwPState <= boostedStates ? maxEnabledSwPState : 0);

				// initiate switching to the temp state and immediately continue with the next core
				WrmsrTx(0xC0010062u, tempSwPState, 0, affinityMask);
			}
		}
		Sleep(3); // let transitions complete
		for (int i = 0; i < _numCores; i++)
		{
			const int& currentHwPState = currentHwPStates[i];
			if (!customized[currentHwPState])
				continue;

			// customize the previously active P-state
			SavePState(currentHwPState, _params.Msrs[currentHwPState][i], i);

			// initiate switching back (or to software P0 if the core was boosted) and immediately
			// continue with the next core
			const int currentSwPState = std::max(0, currentHwPState - boostedStates);
			const DWORD_PTR affinityMask = (DWORD_PTR)1 << i;
			WrmsrTx(0xC0010062u, currentSwPState, 0, affinityMask);
		}
		delete[] currentHwPStates;
		Sleep(3); // let transitions complete

		SetThreadPriority(currentThread, previousPriority);

		// re-enable the Turbo
		if (turboEnabled)
			TurboManager::Set(true);
	}

	/// <summary>
	/// Saves the specified settings for a core's P-state.
	/// </summary>
	static void SavePState(unsigned int index, unsigned int lowMsr, unsigned int core)
	{
		const unsigned int msrIndex = 0xC0010064u + index;
		const DWORD_PTR affinityMask = (DWORD_PTR)1 << core;

		DWORD lower, higher;
		RdmsrTx(msrIndex, &lower, &higher, affinityMask);

		const DWORD lowMsrMask = 0xFE40FFFFu;
		lower = (lower & ~lowMsrMask) | (lowMsr & lowMsrMask);

		WrmsrTx(msrIndex, lower, higher, affinityMask);
	}


	void UnlockPStates()
	{
		DWORD lower, higher;

		// get the max enabled hardware P-state from core 0
		int maxEnabledPState = 4;
		for (int i = 0; i < 5; i++)
		{
			const unsigned int msrIndex = 0xC0010064u + i;
			const DWORD_PTR affinityMask = 1;

			RdmsrTx(msrIndex, &lower, &higher, affinityMask);

			if (!(higher & 0x80000000u) && (!_params.EnableCustomPStates || _params.Msrs[i].empty()))
			{
				maxEnabledPState = (i == 0 ? 0 : i - 1);
				break;
			}
		}

		// make sure the profiles' P-state bounds are valid
		for (int i = 0; i < 3; i++)
		{
			CustomCnQProfile& profile = _params.Profiles[i];

			if (profile.MinPState > maxEnabledPState)
				profile.MinPState = maxEnabledPState;
			if (profile.MaxPState > maxEnabledPState)
				profile.MaxPState = maxEnabledPState;
		}

		// try to set the highest allowed hardware P-state (F3xDC[PstateMaxVal] = maxEnabledPState)
		if (ReadPciConfigDwordEx(0xC3, 0xDC, &lower))
		{
			lower = (lower & 0xFFFFF8FFu) | (maxEnabledPState << 8);
			WritePciConfigDwordEx(0xC3, 0xDC, lower);
		}
	}


	/// <summary>Reads the parameters from the registry.</summary>
	/// <returns>True if successful.</returns>
	bool ReadFromRegistry()
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\BrazosTweaker", 0, KEY_QUERY_VALUE, &_key) != ERROR_SUCCESS)
			return false;

		// custom P-states
		_params.EnableCustomPStates = false;
		for (int i = 0; i < 5; i++)
		{
			const std::string registryValue = std::string("P") + StringUtils::ToString(i);

			std::string text;
			if (Registry::GetString(_key, registryValue, text) && !text.empty())
			{
				if (!ConvertPStateString(_params.Msrs[i], text))
					return false;

				_params.EnableCustomPStates = true;
			}
		}
		bool enableCustomPStates;
		if (Registry::GetBool(_key, "EnableCustomPStates", enableCustomPStates))
		{
			if (!enableCustomPStates)
				_params.EnableCustomPStates = false;
		}

		// Turbo
		_params.TurboCores = -1;
		Registry::GetDword(_key, "TurboCores", _params.TurboCores);

		// custom C&Q
		_params.EnableCustomCnQ = false;
		Registry::GetBool(_key, "EnableCustomCnQ", _params.EnableCustomCnQ);

		// fans
		_params.PwmRamp = 0;
		Registry::GetDword(_key, "PwmRamp", _params.PwmRamp);

		_params.EnableFanManagement = false;
		for (int i = 0; i < 3; i++)
		{
			const std::string registryValue = std::string("Fan") + StringUtils::ToString(i + 1);

			std::string text;
			if (Registry::GetString(_key, registryValue, text))
			{
				if (!ConvertFanCurveString(_params.FanCurves[i], text))
					return false;

				_params.EnableFanManagement = true;
			}
		}

		ReadRegistryProfile("Balanced",         0);
		ReadRegistryProfile("High performance", 1);
		ReadRegistryProfile("Power saver",      2);

		return true;
	}

	void ReadRegistryProfile(const char* subkey, int profileIndex)
	{
		CustomCnQProfile& profile = _params.Profiles[profileIndex];

		// try to open the subkey
		HKEY key = NULL;
		RegOpenKeyEx(_key, subkey, 0, KEY_QUERY_VALUE, &key);

		profile.Ganged = true;
		Registry::GetBool(key, "Ganged", profile.Ganged);

		profile.MinPState = (profileIndex == 2 ? 1 : 0);
		profile.MaxPState = (profileIndex == 1 ? 0 : 4);
		Registry::GetDword(key, "MinPState", profile.MinPState);
		Registry::GetDword(key, "MaxPState", profile.MaxPState);

		profile.SamplingInterval = (profile.Ganged ? 50 : 25);
		profile.ThresholdUp = (profile.Ganged ? 100 / _numCores : 70);
		profile.ThresholdDown = (profile.Ganged ? (int)(0.8f * profile.ThresholdUp) : 20);
		profile.NumSamplesUp = 2;
		profile.NumSamplesDown = 8;
		profile.AggressiveUp = false;
		Registry::GetDword(key, "SamplingInterval", profile.SamplingInterval);
		Registry::GetDword(key, "ThresholdUp", profile.ThresholdUp);
		Registry::GetDword(key, "ThresholdDown", profile.ThresholdDown);
		Registry::GetDword(key, "NumSamplesUp", profile.NumSamplesUp);
		Registry::GetDword(key, "NumSamplesDown", profile.NumSamplesDown);
		Registry::GetBool(key, "AggressiveUp", profile.AggressiveUp);
	}

	/// <summary>
	/// Converts a P-state string to the corresponding 32 lower
	/// bits of one MSR (model-specific register) per core.
	/// </summary>
	/// <param name="msrs">
	/// Receives as many lower 32 bit MSRs as _numCores.
	/// </param>
	/// <returns>True if successful.</returns>
	bool ConvertPStateString(std::vector<unsigned int>& msrs, const std::string& text)
	{
		using namespace std;

		vector<string> tokens;
		StringUtils::Tokenize(tokens, text.c_str(), "|");

		// check if the right number of tokens has been supplied
		if ((int)tokens.size() != _numCores)
			return false;

		for (int i = 0; i < _numCores; i++)
		{
			// parse the hex number
			const unsigned int msr = strtoul(tokens[i].c_str(), NULL, 16);
			msrs.push_back(msr);
		}

		return true;
	}

	bool ConvertFanCurveString(std::vector<std::pair<int, int>>& curve, const std::string& text)
	{
		using namespace std;

		vector<string> pairs;
		StringUtils::Tokenize(pairs, text.c_str(), "|");

		if (pairs.empty())
			return false;

		for (size_t i = 0; i < pairs.size(); i++)
		{
			string& pair = pairs[i];

			// find the pair's delimiter ':'
			const string::size_type delimiterIndex = pair.find(':');
			if (delimiterIndex == string::npos)
				return false;

			string firstString = pair.substr(0, delimiterIndex);
			string secondString = pair.substr(delimiterIndex + 1);

			const int first = atoi(firstString.c_str());
			const int second = atoi(secondString.c_str());

			// make sure the temperatures are sorted ascendingly
			if (i > 0 && first < curve.back().first)
				return false;

			curve.push_back(make_pair(first, second));
		}

		return true;
	}
};
