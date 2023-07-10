# Hockey Slam
This repository contains the full source code and assets for the game Hockey Slam! Hockey Slam is a hockey shootout mobile game developed on Windows and released on Android.

[![Gameplay gif](https://gfycat.com/testypointedemperorpenguin)]

## More info:

[Making of](https://hockeyslam.com/makingof).
[Privacy policy (there's nothing there)](https://hockeyslam.com/privacy).

Apart from a few file loading libraries all the code inside this repository is handwritten. This includes the graphics engine, the memory allocator, physics engine and OpenGL API implementations for both Android and Windows.

The game has been since delisted on the Play Store, however you can download the APK [here.](https://hockeyslam.com/android.apk).

This repository is for anyone curious about game engines or their individual parts. This is not a generic game engine and I suggest not using this for your personal project. The license permits for you to do whatever you want though.

All assets are included, except for the banging tune I couldn't recall the source of.

## Build
1. Install a [MVSC C++ Compiler](https://visualstudio.microsoft.com/vs/features/cplusplus/)
2. Setup your build env with e.g. vcvarsall.bat
3. Make sure your include path contains the OpenGL headers
4. Run `build.bat` from the repository root
5. Copy the `res` folder from the repository root into the `build` folder
6. Run `win_main.exe` in the `build` folder

To run the game in release-mode, set `-DHOKI_DEV=0` in the build script.


