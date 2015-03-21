# Userguide BrazosTweaker #
## Installation ##
To use the tool and it's built-in service, you need to obviously install the given files from the Download section. It could be, that you need to download and install Microsofts .NET Framework 4 upfront, but only in case, it can't be found.
If you already have a previous version of the BrazosTweaker (OntarioTweaker or PhenomMsrTweaker) installed, I strongly suggest to uninstall it first.

BrazosTweaker under Win8
hint from user alejandronova

To install BrazosTweaker in Windows 8, you need to enable the .NET Runtime 2.0. You do that in the "Enable/Disable Windows features" that you'll find in "Programs and Features"

## Tabs ##
Once it has started up, you will see a window six different tabs. The three tabs on the left belong to states, which the CPU can be in, while the next two tabs show the setup for the two available NB (Northbridge) PStates. The last, but not least, is the status tab, which can be used for debug and more detailed info on the internal registers.

## CPU PStates ##
Let's start with the first three tabs.
If you use the E-350 APU, your system uses all three PStates, whereas on the C-50 only two of them are used.
You can see the current active PState by watching the `"*"`. The available PStates are called P0 / P1 / P2. PState P0 is always the fastest mode, while P2 (E-350) is the slowest one. Depending on the current workload of your system, it will switch dydmaically between them. As you can imagine, the P0 is drawing more current than P1 and P2, which is directly related to battery life you can get.

### Voltage ###
With the tool, you can now customize the voltage, which goes into your CPU (CPU VID) for each of the available PStates. Furthermore you can change the divider to get to a different CPU clock than stock.

You can start with the following changes (these are working for most of the users)

  * P0 - 1.225V

  * P1 - 1.05V

  * P2 - 0.85V

If you click "Apply" on the below right, these settings will be temporary set, until you restart or put your system into sleep. You can check the settings by using a well known tool called CPU-Z.

If everything looks alright, you can download Orthos, which is program to do some stress testing. Doing that, tells you, if the system is really stable in most cases. It avoids getting blue screens or freezes later, which can be really annoying.

One should stress each of the PStates separately. This can be achieved by modifying the selecting specific power schemes. On the Lenovo S205, you can use the Energy Management (names might differ):

  * P0 testing -> Power Scheme High Performance
  * P1 testing -> Power Scheme Balanced
  * P2 testing -> Power Scheme Best Battery life

On all other systems, you can do the following:
  1. Press Windows Key and enter "Power Options"
  1. Power Options should open up
  1. On "Balanced" -> click "Change plan settings"
  1. click below Link "Change advanced power settings" on the newly opened page
  1. Scroll down to "Processor power management" and click on the "+"
  1. Now you can play around with Minimum and Maximum processor state (put them to the same value, if you want to have a constant PState)
  1. Note that these values are percentages, i.e. a value of 100% for Min and Max keeps your processor always in P0

Just check, while running the stress test, where the asterisk is! If you want to test P1, the asterisk, should never go to P0, but while stopping the test, it can go to P2.

In case, the systems hangs during testing, you know the selected voltage was too low for that PState. Just simply hold the power button for a few second and your system restarts without the modified voltages.
After restart increase voltage a bit and start stressing the system again. I strongly suggest, not to use a voltage for later use, which is just one step away from, where it hung up. Please add about 25mV at least to have some margin.

Once all that is done and everything runs smoothly, **and only in that case**, you can click on the "Service..." button. Before you read on, please consider looking at this: DeactiveService. By setting that up, there will be a service in the background, which applies the adjusted voltages after coming out of sleep or while booting Win.
You just need to click "Update" to get the current settings, check the selection box "Make custom P-state settings permanent" and hit "Apply".
Now you are done.

### Clock/Divider ###
If you want, you can play around with the dividers to adjust used clocks for each PState. For example, I let my system run for a while on 400MHz, while lowering the voltage to about 0.7V.
Unfortunately you can't overclock by using lower dividers, since the CPU seems to internally lock them to a specific value.
  * Example:
  * E-350 - 1.6GHz
  * FSB 100MHz
  * Multiplier 32x
  * Divider 2
  * Maximum clock 32/2\*100MHz = 1600MHz

Even though you can select a divider smaller than 2 on an E-350, the CPU blocks that and just runs stil at 1.6GHz!

## NB PStates ##

### Voltage ###
Because the NB (Northbridge) and the GPU (graphic unit) are sharing a separate powersupply, it is worth looking into lowering this voltage in addition to gain some more battery life.
Basically the adjustments you can do, need to be done the same way as for the CPU, except that only two PStates are available.
On my system, the following settings work stable:
  * NB P0 - 0.85V
  * NB P1 - 0.8V

Just a little lower and it hangs. Just press the Power button for a while and it will get back to life.

### Clock/Divider ###
There is no opportunity to mess with the dividers on the NB so far.

If you like this application and you want to support my work, please donate.<br>
<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=KDVJC4359EN64'><img src='https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif' /></a>