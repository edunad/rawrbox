<p align="center">
  <img src="https://i.rawr.dev/caw_sleep.png" width=215 /><br/>
</p>

<h2>
	<p align="center">
		<a href="https://github.com/MythicalRawr/rawrbox">RawrBox</a> - Another modular(?) game engine made with <a href="https://github.com/bkaradzic/bgfx">BGFX</a>
	</p>
</h2>

<p align="center">
	<img src="https://github.com/edunad/rawrbox/actions/workflows/build.yml/badge.svg?branch=master&event=push"/>
</p>

<h4>
	<p align="center">
		Similar to <a href="https://github.com/Goofy-Penguin/Mainframe">Mainframe</a> but with focus on games (and so i can practice some C++)
	</p>
	<p align="center">
		<a href="#features">Features</a> -
		<a href="#libs">Libs</a> -
		<a href="#cmake">Settings</a> -
		<a href="#dependencies">Dependencies</a> -
		<a href="#building">Building</a>
	</p>
</h4>

# FEATURES (so far)

-   Model loading (animations, light, texture import)
-   Light support (point, directional, spotlight)
-   3D text
-   2D stencil drawing
-   Font loading
-   Image loading (png, jpeg, tga, bitmap)
-   Sound loading (using BASS)
-   Custom UI
-   GIF loading / animation
-   Vulkan / DirectX / OpenGL support
-   Wayland support (linux only)
-   Works on steam deck 😺
-   Physics system using [JoltPhysics](https://github.com/jrouwe/JoltPhysics)

# TODO LIST

-   Decal system
-   Add animation blending
-   Add shadow maps to lights
-   Add scripting lib (with lua support)
-   Add steam networking lib

# LIBS

| LIB                 | DESCRIPTION                                        | USAGE                                                                                      |      DEPENDENCIES      |
| :------------------ | :------------------------------------------------- | :----------------------------------------------------------------------------------------- | :--------------------: |
| `RAWRBOX.RENDER`    | Rendering lib (aka, contains window, stencil, etc) | Contains window, stencil, model / texture loading. Basically anything related to rendering |   `ENGINE` & `MATH`    |
| `RAWRBOX.MATH`      | Math lib                                           | Contains vector, color and other math related classes                                      |                        |
| `RAWRBOX.BASS`      | Bass lib (aka sound loading)                       | Loads sounds using the BASS lib, supports 3D & http sound streaming                        |   `ENGINE` & `MATH`    |
| `RAWRBOX.UTILS`     | Utils lib                                          | Utils for game development                                                                 |                        |
| `RAWRBOX.ENGINE`    | Engine lib (aka game loop)                         | The engine it self, contains the game loop mechanism                                       |        `UTILS`         |
| `RAWRBOX.DEBUG`     | Debug utils lib                                    | Automatically adds GIZMOS and other debug methods to the renderer                          |        `RENDER`        |
| `RAWRBOX.UI`        | UI lib                                             | UI components lib                                                                          | `RENDER` & `RESOURCES` |
| `RAWRBOX.RESOURCES` | Resources manager lib                              | Resource manager. Handles loading and stores the pointers for easy access                  |        `UTILS`         |
| `RAWRBOX.PHYSICS`   | Physics lib                                        | Physics lib                                                                                |   `ENGINE` & `MATH`    |

<p align="center">
  <img src="./RAWBOX%20-%20Dependencies.png" />
</p><br/>

# CMAKE OPTIONS

| OPTION NAME                                | NOTE                                                                                               |
| :----------------------------------------- | :------------------------------------------------------------------------------------------------- |
| `RAWRBOX_OUTPUT_BIN`                       | The output build folder. Default is `bin`                                                          |
| `RAWRBOX_CONTENT_FOLDER`                   | The content folder to output resources. Default is `content`                                       |
| --                                         | --                                                                                                 |
| `RAWRBOX_USE_WAYLAND`                      | Enables WAYLAND compiling on LINUX                                                                 |
| --                                         | --                                                                                                 |
| `RAWRBOX_ENABLE_QHULL`                     | Enables QHull util                                                                                 |
| `RAWRBOX_ENABLE_ASSIMP_SUPPORT`            | Enables assimp model loading                                                                       |
| `RAWRBOX_ENABLE_BASS_SUPPORT`              | Enables BASS support. ⚠️ [BASS IS ONLY FREE FOR OPEN SOURCE PROJECTS](https://www.un4seen.com/) ⚠️ |
|                                            |                                                                                                    |
| `RAWRBOX_BUILD_SAMPLES`                    | Builds the project sample                                                                          |
| `RAWRBOX_BUILD_TESTING`                    | Builds and runs tests                                                                              |
| `RAWRBOX_BUILD_UI`                         | Builds and includes ui                                                                             |
| `RAWRBOX_BUILD_DEBUG`                      | Builds and includes debug (aka gizmo rendering & renderdoc)                                        |
| `RAWRBOX_BUILD_RESOURCES_MANAGER`          | Builds and resouces manager (aka handling and storing loaded resources)                            |
| `RAWRBOX_BUILD_PHYSICS`                    | Builds the physics engine                                                                          |
|                                            |                                                                                                    |
| `RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME` | Builds libraries with MSVC Multithreaded runtime                                                   |

# DEPENCENDIES

| LIB             | REQUIRED | NOTE                                                |
| :-------------- | :------: | :-------------------------------------------------- |
| bgfx            |    ✔️    |                                                     |
| bx              |    ✔️    | Required by `bgfx`                                  |
| glfw            |    ✔️    |                                                     |
| nlohmann_json   |    ✔️    | Used for loading JSON files                         |
| fmt             |    ✔️    | Used for formatting                                 |
| utfcpp          |    ✔️    | Used for text rendering                             |
| stb/image       |    ✔️    | Used for loading images                             |
| stb/image_write |    ✔️    | Used for writting images                            |
| stb/freetype    |    ✔️    | Used for loading fonts                              |
| thread-pool     |    ✔️    | Used for managing async                             |
| qhull           |    ✖️    | Used for calculating convex hulls from given points |
| assimp          |    ✖️    | Used for loading models                             |
| catch2          |    ✖️    | Used for testing                                    |
| bass            |    ✖️    | Used for loading sounds                             |
| JoltPhysics     |    ✖️    | Used for managing physics                           |

# BUILDING

## WINDOWS

### REQUIRED SOFTWARE

-   [GIT](https://git-scm.com/) or something similar to GIT
-   Download and install **C++ Build Tools**
-   Download and install a IDE, [Visual Code](https://code.visualstudio.com/) is **recommended**.
-   Download and setup [ninja](https://github.com/ninja-build/ninja/releases) (make sure it's in the enviroment path)

### IDE SETUP (Visual Code)

-   Open the workspace file and install the recommended extensions (at least the C++, CMAKE)
-   Run cmake configure
-   Select `[Visual Studio Build Tools <version> Release - amd64]` on the kit popup (for all of the projects)
-   Run the compiler by hitting F5
-   Hope it builds fine 🙏

## LINUX

### REQUIRED SOFTWARE

-   [PODMAN](https://podman.io/)

### IDE SETUP

-   TODO
