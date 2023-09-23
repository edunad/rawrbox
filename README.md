<h2>
	<p align="center">
		<img src="./rawrbox.svg" width=300 /><br/>
		<a href="https://github.com/edunad/rawrbox">RawrBox</a> - PSX..ish game engine
	</p>
</h2>

<h4>
	<p align="center">
		<img src="https://github.com/edunad/rawrbox/actions/workflows/build.yml/badge.svg?branch=master&event=push"/>&nbsp;
		<img src="https://img.shields.io/github/license/edunad/rawrbox.svg"/>&nbsp;
		<img src="https://img.shields.io/github/issues/edunad/rawrbox/BUG.svg"/>&nbsp;
		<img src="https://img.shields.io/github/last-commit/edunad/rawrbox.svg"/>&nbsp;
		<a href="https://box.rawr.dev"><img src="https://img.shields.io/badge/wiki-%F0%9F%90%B2-black"/></a>
		<br/><br/>
		<a href="#features">Features</a> -
		<a href="#libs">Libs</a> -
		<a href="#cmake">Settings</a> -
		<a href="#dependencies">Dependencies</a> -
		<a href="#building">Building</a> -
		<a href="#samples">Samples</a>
	</p>
</h4>

# FEATURES

- Model loading (animations, light, texture, blend shapes) using [assimp](https://github.com/assimp/assimp)
- Light support (point, spot, directional)
- Clustered rendering & support for other type of rendering
- 3D text
- 2D stencil drawing
- Fonts (ttf) using [stb](https://github.com/nothings/stb)
- JSON loading using [nlohmann json](https://github.com/nlohmann/json)
- Images loading (png, jpeg, tga, bitmap, gif) using [stb](https://github.com/nothings/stb), SVG using [lunasvg](https://github.com/sammycage/lunasvg), WEBP using [libwebp](https://github.com/webmproject/libwebp)
- Video only using [webm](https://github.com/webmproject/libwebm)
- Sounds using [BASS](https://www.un4seen.com/) lib
- Custom UI
- Material system (similar to unity)
- Vulkan / DirectX / OpenGL support using [BGFX](https://github.com/bkaradzic/bgfx)
- Linux Wayland support + steam deck support
- 3D Physics system using [JoltPhysics](https://github.com/jrouwe/JoltPhysics)
- 2D Physics system using [Muli](https://github.com/Sopiro/Muli)
- Decal system
- Instancing support
- Scripting / modding support with LUA using [sol2](https://github.com/ThePhD/sol2) and [luajit2](https://github.com/openresty/luajit2)
- Post-processing effects support
- HTTP / HTTPS requests using curl + [libcpr](https://github.com/libcpr/cpr)
- Packet networking support
- Settings support
- Custom dev console (with support for custom commands)
- I18N (internationalization) support
- GPU picking

# TODO LIST

- Add animation blending
- Add lights shadow maps

# LIBS

| LIB                  | DESCRIPTION                                        | USAGE                                                                                      |      DEPENDENCIES      |
| :------------------- | :------------------------------------------------- | :----------------------------------------------------------------------------------------- | :--------------------: |
| `RAWRBOX.RENDER`     | Rendering lib (aka, contains window, stencil, etc) | Contains window, stencil, model / texture loading. Basically anything related to rendering |   `ENGINE` & `MATH`    |
| `RAWRBOX.MATH`       | Math lib                                           | Contains vector, color and other math related classes                                      |                        |
| `RAWRBOX.BASS`       | Bass lib (aka sound loading)                       | Loads sounds using the BASS lib, supports 3D & http sound streaming                        |   `ENGINE` & `MATH`    |
| `RAWRBOX.UTILS`      | Utils lib                                          | Utils for game development                                                                 |                        |
| `RAWRBOX.ENGINE`     | Engine lib (aka game loop)                         | The engine it self, contains the game loop mechanism                                       |        `UTILS`         |
| `RAWRBOX.UI`         | UI lib                                             | UI components lib                                                                          | `RENDER` & `RESOURCES` |
| `RAWRBOX.RESOURCES`  | Resources manager lib                              | Resource manager. Handles loading and stores the pointers for easy access                  |        `UTILS`         |
| `RAWRBOX.PHYSICS.3D` | 3D Physics lib                                     | Handles 3D physics                                                                         |   `ENGINE` & `MATH`    |
| `RAWRBOX.PHYSICS.2D` | 2D Physics lib                                     | Handles 2D physics                                                                         |   `ENGINE` & `MATH`    |
| `RAWRBOX.ASSIMP`     | Assimp lib                                         | Handles assimp model loading                                                               | `RENDER` & `RESOURCES` |
| `RAWRBOX.WEBM`       | WEBM lib                                           | Handles webm loading                                                                       | `RENDER` & `RESOURCES` |
| `RAWRBOX.SCRIPTING`  | Scripting lib                                      | Handles lua & mod loading                                                                  |   `ENGINE` & `MATH`    |
| `RAWRBOX.NETWORK`    | Networking lib                                     | Handles networking                                                                         |                        |

<br/><br/>

# CMAKE OPTIONS

| OPTION NAME                                      | NOTE                                                                                                      |
| :----------------------------------------------- | :-------------------------------------------------------------------------------------------------------- |
| `RAWRBOX_BUILD_TESTING`                          | Builds & enables project tests                                                                            |
| --                                               | --                                                                                                        |
| `RAWRBOX_OUTPUT_BIN`                             | The output build folder. Default is `bin`                                                                 |
| `RAWRBOX_CONTENT_FOLDER`                         | The content folder to output resources. Default is `content`                                              |
| `RAWRBOX_SHADERS_FOLDER`                         | The content folder to output shaders. Default is `shaders`                                                |
| --                                               | --                                                                                                        |
| `RAWRBOX_COPY_SHADER_INCLUDES`                   | Copy shader includes into your `RAWRBOX_SHADERS_FOLDER` folder (useful if overriding rawrbox shaders)     |
| --                                               | --                                                                                                        |
| `RAWRBOX_USE_WAYLAND`                            | Enables WAYLAND compiling on LINUX                                                                        |
| --                                               | --                                                                                                        |
| `RAWRBOX_BUILD_QHULL`                            | Builds QHull util                                                                                         |
| `RAWRBOX_BUILD_SAMPLES`                          | Builds the project sample                                                                                 |
| `RAWRBOX_BUILD_RAWRBOX_UI`                       | Builds and includes ui                                                                                    |
| `RAWRBOX_BUILD_RAWRBOX_RESOURCES`                | Builds and resouces manager (aka handling and storing loaded resources)                                   |
| `RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS`               | Builds the 3D physics engine                                                                              |
| `RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS`               | Builds the 2D physics engine                                                                              |
| `RAWRBOX_BUILD_RAWRBOX_NETWORK`                  | Builds network support                                                                                    |
| `RAWRBOX_BUILD_RAWRBOX_BASS`                     | Enables BASS support. ⚠️ [BASS IS ONLY FREE FOR OPEN SOURCE PROJECTS](https://www.un4seen.com/) ⚠️        |
| `RAWRBOX_BUILD_RAWRBOX_ASSIMP`                   | Enables assimp model loading                                                                              |
| `RAWRBOX_BUILD_RAWRBOX_WEBM`                     | Enables WEBM loading                                                                                      |
| --                                               | --                                                                                                        |
| `RAWRBOX_DEV_MODE`                               | Enables all the modules, used for rawrbox development                                                     |
| --                                               | --                                                                                                        |
| `RAWRBOX_BUILD_RAWRBOX_SCRIPTING`                | Enables lua & modding support                                                                             |
| ~~`RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT`~~ | ~~Enable scripting LuaJIT support~~ (Currently broken [#73](https://github.com/edunad/rawrbox/issues/73)) |
| --                                               | --                                                                                                        |
| `RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME`       | Builds libraries with MSVC Multithreaded runtime (Auto-enabled if jolt is used)                           |

<br/><br/>

# INTERNAL DEPENDENCIES

<br/><br/>

<p align="center">
  <img src="./RAWBOX%20-%20Dependencies.png" />
</p>

<br/><br/><br/>

# EXTERNAL DEPENDENCIES

| LIB                 | NOTE                                                |
| :------------------ | :-------------------------------------------------- |
| bgfx                |                                                     |
| bx                  | Required by `bgfx`                                  |
| glfw                |                                                     |
| nlohmann_json       | Used for JSON loading                               |
| fmt                 | Used for text formatting                            |
| utfcpp              | Used for utf support                                |
| stb/image           | Used for loading images (png,gif,jpeg)              |
| stb/image_write     | Used for writting images                            |
| stb/freetype        | Used for loading fonts                              |
| thread-pool         | Used for managing async threading                   |
| qhull               | Used for calculating convex hulls from given points |
| assimp              | Used for loading models                             |
| catch2              | Used for testing                                    |
| bass                | Used for loading sounds                             |
| JoltPhysics         | Used for managing 3D physics                        |
| muli                | Used for managing 2D physics                        |
| webmlib & vpx       | Used for loading webm                               |
| lua & sol2 & luajit | Used for loading & managing lua scripting           |
| curl + libcpr       | Used for HTTP / HTTPS requests                      |
| lunasvg             | Used for SVG loading                                |
| libwebp             | Used for WEBP loading                               |

# BUILDING

## WINDOWS

### REQUIRED SOFTWARE

- [GIT](https://git-scm.com/) or something similar to GIT
- [CMAKE](https://cmake.org/download/) at least > 3.16.3
- Download and install **C++ Build Tools** (2022 is recommended)<br/>
  - ![](https://i.rawr.dev/KcTNzFJZhZ.png)
- Download and install a IDE, [Visual Code](https://code.visualstudio.com/) is **recommended**.
- Download and setup [ninja](https://github.com/ninja-build/ninja/releases) (make sure it's in the enviroment path)<br/>
  - `pip install ninja` if you have python3 installed

### IDE SETUP (Visual Code)

- Open the workspace file (`.code-workspace`) and install the recommended extensions (at least **C++** & **CMAKE**)
- Run **cmake configure**
- Select `[Visual Studio Build Tools <version> Release - amd64]` on the kit popup (for all of the projects)
- Run the compiler by hitting F5

## LINUX

### REQUIRED SOFTWARE

- [PODMAN](https://podman.io/)

### IDE SETUP

- TODO

# SAMPLES

### 001-stencil

<a href='/samples/001-stencil'><img src="https://i.rawr.dev/wBpBiDBvCt-min.gif" width=512 /></a>

### 002-generated-models

<a href='/samples/002-generated-models'><img src="https://i.rawr.dev/XelPGoCABW-min.gif" width=512 /></a>

### 003-assimp

<a href='/samples/003-assimp'><img src="https://i.rawr.dev/L0rZ8jYwkH-min.gif" width=512 /></a>

### 004-light-support

<a href='/samples/004-light-support'><img src="https://i.rawr.dev/Xvk5DpQPUd-min.gif" width=512 /></a>

### 005-post-process

<a href='/samples/005-post-process'><img src="https://i.rawr.dev/4YBkWan9pe-min.gif" width=512 /></a>

### 006-bass-loading

<a href='/samples/006-bass-loading'><img src="https://i.rawr.dev/a0AeGGNQ7A-min.gif" width=512 /></a>

### 007-particle-system

<a href='/samples/007-particle-system'><img src="https://i.rawr.dev/55dUuFJqKR-min.gif" width=512 /></a>

### 008-ui

<a href='/samples/008-ui'><img src="https://i.rawr.dev/otAM4cyFtH-min.gif" width=512 /></a>

### 009-physics

<a href='/samples/009-physics-3d'><img src="https://i.rawr.dev/Ir57QELn6F-min3.gif" width=512 /></a>

### 010-instancing

<a href='/samples/010-instancing'><img src="https://i.rawr.dev/dwPPjtAdzr-min.gif" width=512 /></a>

### 011-decals

<a href='/samples/011-decals'><img src="https://i.rawr.dev/x4ekTeLHzu-min.gif" width=512 /></a>

### 012-webm

<a href='/samples/012-webm'><img src="https://i.rawr.dev/sH0bFsxJhp-min.gif" width=512 /></a>

### 013-physics-2d

<a href='/samples/013-physics-2d'><img src="https://i.rawr.dev/YzkRS6BO30-min.gif" width=512 /></a>

### 014-scripting

<a href='/samples/014-scripting'><img src="https://i.rawr.dev/EgZILBUsED-min.gif" width=512 /></a>
