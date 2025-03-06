### **CEER**  
***Halo CE Enemy Randomiser***

Randomises enemies in Halo CE for MCC. 
Can also multiply enemies.
And randomise textures.
And randomise sounds. 

Grab the latest release [here.](https://github.com/Burnt-o/CEER/releases)

More info in this video [here.](https://youtu.be/14dXUNCHrUc)

Custom Campaign files if you want to play with every enemy on every level [here](https://steamcommunity.com/sharedfiles/filedetails/?id=2990052285) (not necessary but highly recommended). Season 8 campaign files if you're downpatching are [here](https://mega.nz/file/K2QX0YrL#i7tFkOA_aKHzeLVkUNoo5UHesXpCRvq99QCgBbSvDPk).

Remember to run MCC with anticheat disabled.

Supports current patch (v3495) as of Feb 2025, as well as Season 8 (v2645).


COOP:
To play coop, both players must use identical settings (and with a set seed, not an empty one). The copy & paste functions can help you set this up. You also want to get all your settings set while in the pre-game lobby, as changing settings mid-game is likely to cause desync.

**Building from source:**
To build the source yourself you'll need the following libraries from vcpkg. 
 * plog:x64-windows
 * plog:x64-windows-static
 * eventpp:x64-windows-static
 * imgui\[dx11-binding,win32-binding\]:x64-windows-static
 * pugixml:x64-windows-static
 * curl\[non-http,schannel,ssl,sspi\]:x64-windows-static
 * boost-stacktrace:x64-windows-static
 * boost-algorithm:x64-windows-static
 
This project also makes heavy use of the amazing [SafetyHook](https://github.com/cursey/safetyhook) by cursey. You can think of it as like "Microsoft Detours but if it didn't suck". I've included the release I'm using in the solution files. 
You'll also need the Windows 10 SDK.
