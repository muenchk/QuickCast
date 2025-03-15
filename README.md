This is the Source code to the Skyrim Mod [QuickCast]().
Bug reports and suggestions should be posted in the Bug / Forum section of the nexusmods page.

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [The Elder Scrolls V: Skyrim Special Edition](https://store.steampowered.com/app/489830)
	* Add the environment variable `SkyrimOutputPath` to point to the folder where you want to store your compiled Plugins.
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++
* [CommonLibNG](https://github.com/CharmedBaryon/CommonLibSSE-NG)
	* add its path to the environment variable `CommonLibNGPath`

    
## User Requirements
* [Skyrim Script Extender](https://skse.silverlock.org/)
* [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
	* For the Special Edition or Anniversary Edition

    
## Building for SSE / AE
```
git clone https://github.com/muenchk/QuickCast.git
cd NPCsUsePotions
cmake --preset all 				
cmake --build build --config Release
```