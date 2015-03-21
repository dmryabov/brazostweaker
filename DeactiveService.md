# Why do I need to know this? #

This page describes, what to do, in case your system doesn't boot into Windows anymore, because of the service.

# Details #

  1. In order to disable the service, you need to boot Windows in "Safe mode". While doing this, the BrazosTweaker service doesn't get enabled.
  1. If Windows has booted succesfully in "Safe mode", you can click on the Windows-Start button and enter "Services" in the below line. Hit "Enter".
  1. Now you can see all installed services and should look for "BrazosTweaker service".
  1. The next thing is clicking right on the line from BrazosTweaker and select "Properties".
  1. On the most left tab (General?) look for line "Startup type" (or something similar). Switch from "Automatic" to "Manual".
  1. Now you can restart your system.
  1. Once Windows has been restarted, start BrazosTweaker and try to find the rootcause of the hangs (too low voltage for a certain PState?, bug in the code, etc.).
  1. If the problem was found and fixed, you can re-enable the service by starting with 2. and switching back to "Automatic".