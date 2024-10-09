<h2>
	<p align="center">
		<img alt="logo" src="./rawrbox.svg" width="300" /><br/>
		<a href="https://github.com/edunad/rawrbox">RawrBox</a> - PSX..ish game engine
	</p>
</h2>
<h4>
	<p align="center">
		<a href="https://github.com/edunad/rawrbox/actions"><img alt="logo" src="https://github.com/edunad/rawrbox/actions/workflows/build.yml/badge.svg?branch=master&event=push"/>&nbsp;</a>
		<a href="/LICENSE"><img alt="logo" src="https://img.shields.io/github/license/edunad/rawrbox.svg"/>&nbsp;</a>
		<a href="https://github.com/edunad/rawrbox/issues?q=is%3Aopen+is%3Aissue+label%3ABUG"><img alt="logo" src="https://img.shields.io/github/issues/edunad/rawrbox/BUG.svg"/>&nbsp;</a>
		<a href="https://github.com/edunad/rawrbox/commits/master/"><img alt="logo" src="https://img.shields.io/github/last-commit/edunad/rawrbox.svg"/>&nbsp;</a><br/>
		<a href="https://app.codacy.com/gh/edunad/rawrbox/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade"><img alt="logo" src="https://app.codacy.com/project/badge/Grade/d8f8badc1e654f2a8098e3a624405bbd"/>&nbsp;</a>
		<a href="https://box.rawr.dev"><img src="https://img.shields.io/badge/wiki-%F0%9F%90%B2-black"/></a>
		<a href="https://discord.gg/cHWmjEn2yg"><img src="https://img.shields.io/discord/1268630826070052885?logo=discord&label=Discord"/></a>
		<br/><br/>
		<a href="#features">Features</a> -
		<a href="#libs">Libs</a> -
		<a href="#building">Building</a> -
		<a href="#samples">Samples</a>
	</p>
</h4>

<p align="center">
	<a href="https://github.com/edunad/rawrbox-template"><img alt="template" src="./RAWRBOX-TEMPLATE.png" width="256" /></a><br/>
</p>

## Why PSXish?

This engine started as a C++ training project, with hopes of being applied in my own games. Don't expect it to rival big players like Unity, Unreal or Godot; it's not geared towards advanced rendering techniques such as anti-aliasing or ray-tracing. Rather, it's tailored for creating PlayStation 1-style games.

> [!IMPORTANT]
> World Coordinates: [left handed](<https://learn.microsoft.com/en-us/previous-versions/windows/desktop/images/bb324490.leftrght(en-us,vs.85).gif>).<br/>
> Matrix: [column-major](https://upload.wikimedia.org/wikipedia/commons/thumb/4/4d/Row_and_column_major_order.svg/1200px-Row_and_column_major_order.svg.png).

# FEATURES

|    Module     | Features                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| :-----------: | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|  **Engine**   | - C++23<br/>- Tick based system<br/>- Limit FPS and TPS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| **Animation** | - Skeleton data support<br/> - Blendshapes support<br/> - Multiple armature animation support<br/> - Vertex animation                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
|   **Data**    | - File formats: <br/>&nbsp;&nbsp;&nbsp;&nbsp;- Meshes: <br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- GLTF 2.0 (using [fastgltf](https://github.com/spnda/fastgltf))<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- OBJ (using ???)<br/>- Images: <br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- PNG, JPEG, BMP, TGA, GIF (using [stb](https://github.com/nothings/stb))<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- SVG (using [lunasvg](https://github.com/sammycage/lunasvg))<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- WEBP (using [libwebp](https://github.com/webmproject/libwebp))<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- WEBM (video only) (using [libwebm](https://github.com/webmproject/libwebm))<br/>- Fonts: <br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- TTF (using [stb](https://github.com/nothings/stb))<br/>- Audio (using [BASS](https://www.un4seen.com/)): <br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- OGG<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- MP4<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- MP3<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- WAV<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Supports url audio<br/>- JSON (using [glaze](https://github.com/stephenberry/glaze)) |
|   **Math**    | - Vectors, colors, matrices & quaternions support<br/> - Bezier curve & easing<br> - Packing support (float16, etc)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|  **Physics**  | - 3D multi-threaded physics (using [JoltPhysics](https://github.com/jrouwe/JoltPhysics))<br/> - 2D physics support (using [Muli](https://github.com/Sopiro/Muli))                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| **Rendering** | - **Vulkan** & **DirectX 12** support using ([Diligent](https://github.com/DiligentGraphics/DiligentCore))<br/>- Material system similar to unity<br/>- Window & Input callbacks (using [GLFW](https://github.com/glfw/glfw)) <br/>- Clustered rendering :<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Decals (**WIP**)<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Lights (point / spot/ directional)<br/>- Linux (**steamdeck**) support<br/>- Bindless rendering<br/>- Post-processing support<br/> - 2D rendering<br/> - Custom rendering plugins<br/> - GPU Picking<br/> - [IMGUI](https://github.com/ocornut/imgui) support<br/> - GPU Particle system<br/>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
|  **Network**  | - HTTP / HTTPS requests (using [libcpr](https://github.com/libcpr/cpr))<br/>- Packet network implementation<br/>- **NetVar** & **NetArray** support for [snapshot](https://gafferongames.com/post/snapshot_interpolation/) implementation <br/>- Simple socket implementation                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| **Scripting** | - Scripting / modding support with LUAU (using [luabridge3](https://github.com/kunitoki/LuaBridge3) + [luau](https://github.com/luau-lang/luau))                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
|   **Other**   | - I18N (internationalization) support<br/>- Settings support<br/>- Steamworks support<br/>- Custom UI<br/>- DEV Console with support for custom commands & scripts<br/> - Timer utils (based on [GMOD's](https://wiki.facepunch.com/gmod/timer) timers)<br/>- File watcher implementation <br/>- Threading utilities                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |

# SAMPLES

|        001-stencil<br/><a href='/samples/001-stencil'><img src="https://i.rawr.dev/sample1-min-4.gif" width="240" /></a>        | 002-generated-models<br/><a href='/samples/002-generated-models'><img src="https://i.rawr.dev/sample2-min-4.gif" width="240" /></a> |       003-light<br/><a href='/samples/003-light'><img src="https://i.rawr.dev/sample3-min-4.gif" width="240" /></a>        |
| :-----------------------------------------------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------------------------------------------: | :------------------------------------------------------------------------------------------------------------------------: |
|     004-instancing<br/><a href='/samples/004-instancing'><img src="https://i.rawr.dev/sample4-min-2.gif" width="240" /></a>     |     005-post-process<br/><a href='/samples/005-post-process'><img src="https://i.rawr.dev/sample5-min-2.gif" width="240" /></a>     |      006-decals<br/><a href='/samples/006-decals'><img src="https://i.rawr.dev/sample6-min-2.gif" width="240" /></a>       |
| 007-particle-system<br/><a href='/samples/007-particle-system'><img src="https://i.rawr.dev/sample7-min.gif" width="240" /></a> |               008-ui<br/><a href='/samples/008-ui'><img src="https://i.rawr.dev/sample8-min-5.gif" width="240" /></a>               |        009-gltf<br/><a href='/samples/009-gltf'><img src="https://i.rawr.dev/sample9-min-4.gif" width="240" /></a>         |
|     010-bass-audio<br/><a href='/samples/010-bass-audio'><img src="https://i.rawr.dev/sample10-min.gif" width="240" /></a>      |      011-physics-3D<br/><a href='/samples/011-physics-3D'><img src="https://i.rawr.dev/sample11-min-2.gif" width="240" /></a>       |  012-physics-2D<br/><a href='/samples/012-physics-2D'><img src="https://i.rawr.dev/sample12-min-2.gif" width="240" /></a>  |
|          013-webm<br/><a href='/samples/013-webm'><img src="https://i.rawr.dev/sample13-min-2.gif" width="240" /></a>           |       014-scripting<br/><a href='/samples/014-scripting'><img src="https://i.rawr.dev/sample14-min-2.gif" width="240" /></a>        | 015-gpu-picking<br/><a href='/samples/015-gpu-picking'><img src="https://i.rawr.dev/sample15-min-2.gif" width="240" /></a> |
|     016-steamworks<br/><a href='/samples/016-steamworks'><img src="https://i.rawr.dev/sample16-min.gif" width="240" /></a>      |           017-imgui<br/><a href='/samples/017-imgui'><img src="https://i.rawr.dev/sample17-min-2.gif" width="240" /></a>            |                                                                                                                            |

# BUILDING

## WINDOWS

### REQUIRED SOFTWARE

- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) 2015 - 2022 version
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (ensure the enviroment paths are correct)
- [GIT](https://git-scm.com/) or something similar to GIT
- [CMAKE](https://cmake.org/download/) at least > 3.16.3
- Download and install **C++ Build Tools** (2022 is recommended)<br/>
  - ![](https://i.rawr.dev/KcTNzFJZhZ.png)
- Download and install a IDE, [Visual Code](https://code.visualstudio.com/) is **recommended**.
- Download and setup [ninja](https://github.com/ninja-build/ninja/releases) (make sure it's in the enviroment path)<br/>
  - `pip install ninja` if you have python3 installed
- C++23 is required to build.

### IDE SETUP (Visual Code)

1. Open the workspace file (`.code-workspace`) and install the recommended extensions (at least **C++** & **CMAKE**)
2. Run **cmake configure**
3. Select `[Visual Studio Build Tools <version> Release - amd64]` on the kit popup (for all of the projects)
4. Run the compiler by hitting F5

## LINUX

### REQUIRED SOFTWARE

- [PODMAN](https://podman.io/) or [DOCKER](https://docker.com/)

### SETUP

1. Build the container : `podman build -t rawrbox .`
2. Run the container : `podman run -ti rawrbox /bin/bash`
3. Run `cmake_rawrbox` (ALIAS) to prepare the cmake rawrbox
4. Run `build_rawrbox` (ALIAS) to build rawrbox

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
| `RAWRBOX.GLTF`       | GLTF lib                                           | Handles gltf2.0 model loading                                                              | `RENDER` & `RESOURCES` |
| `RAWRBOX.WEBM`       | WEBM lib                                           | Handles webm loading                                                                       | `RENDER` & `RESOURCES` |
| `RAWRBOX.SCRIPTING`  | Scripting lib                                      | Handles lua & mod loading                                                                  |   `ENGINE` & `MATH`    |
| `RAWRBOX.NETWORK`    | Networking lib                                     | Handles networking                                                                         |                        |
| `RAWRBOX.STEAMWORKS` | Steamworks lib                                     | Handles steamworks                                                                         |        `UTILS`         |
| `RAWRBOX.IMGUI`      | IMGUI lib                                          | Handles imgui                                                                              |        `RENDER`        |

<br/><br/>

# CMAKE OPTIONS

| OPTION NAME                                | NOTE                                                                                               | DEFAULT |
| :----------------------------------------- | :------------------------------------------------------------------------------------------------- | ------- |
| `RAWRBOX_BUILD_TESTING`                    | Builds & enables project tests                                                                     | OFF     |
| `RAWRBOX_BUILD_SAMPLES`                    | Builds the project sample                                                                          | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_CONTENT_FOLDER`                   | The content folder to output assets. Default is `assets`                                           | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_USE_WAYLAND`                      | Enables WAYLAND compiling on LINUX                                                                 | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_BUILD_RAWRBOX_RENDER`             | Builds and the renderer, disable for renderless programs                                           | ON      |
| `RAWRBOX_BUILD_RAWRBOX_UI`                 | Builds rawrbox ui (alternative to imgui)                                                           | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_RESOURCES`          | Builds the resouces manager (aka handling and storing loaded resources)                            | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS`         | Builds the 3D physics engine                                                                       | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS`         | Builds the 2D physics engine                                                                       | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_BASS`               | Enables BASS support. ⚠️ [BASS IS ONLY FREE FOR OPEN SOURCE PROJECTS](https://www.un4seen.com/) ⚠️ | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_GLTF`               | Enables gltf2.0 model loading support                                                              | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_WEBM`               | Enables WEBM loading                                                                               | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_NETWORK`            | Builds network support                                                                             | OFF     |
| `RAWRBOX_BUILD_RAWRBOX_IMGUI`              | Builds imgui support                                                                               | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_BUILD_RAWRBOX_SCRIPTING`          | Enables lua & modding support                                                                      | OFF     |
| `RAWRBOX_SCRIPTING_UNSAFE`                 | Enables io support on lua (loading and saving files on the data folder)                            | OFF     |
| `RAWRBOX_SCRIPTING_EXCEPTION`              | Enables scripting throwing exceptions instead of catching them                                     | OFF     |
| `RAWRBOX_SCRIPTING_WORKSHOP_MODDING`       | Enables workshop utilities (useful for steam workshop / mod.io)                                    | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_BUILD_RAWRBOX_STEAMWORKS`         | Enables steamworks support                                                                         | OFF     |
| `STEAMWORKS_APPID`                         | Sets the steamworks appid                                                                          | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_BUILD_QHULL`                      | Builds QHull util                                                                                  | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME` | Builds libraries with MSVC Multithreaded runtime (Auto-enabled if jolt is used)                    | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_DISABLE_SUPPORT_DX12`             | Disable dx12 support                                                                               | OFF     |
| `RAWRBOX_DISABLE_SUPPORT_VULKAN`           | Disable vulkan support                                                                             | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_DEV_MODE`                         | Enables all the modules, used for rawrbox development                                              | OFF     |
| --                                         | --                                                                                                 | --      |
| `RAWRBOX_INTERPROCEDURAL_OPTIMIZATION`     | Enables IPO compilation on release                                                                 | ON      |

<br/><br/>

# INTERNAL DEPENDENCIES

<br/><br/>

<p align="center">
  <img alt="dependencies" src="./RAWBOX%20-%20Dependencies.png" />
</p>

<br/><br/><br/>

# EXTERNAL DEPENDENCIES

| LIB               | NOTE                                                | LICENSE                                                                                        |
| :---------------- | :-------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| DiligentCore      | Used for vulkan / directx rendering                 | [Apache License 2.0](https://github.com/DiligentGraphics/DiligentCore/blob/master/License.txt) |
| glfw              | Used for window management                          | [ZLIB](https://github.com/glfw/glfw/blob/master/LICENSE.md)                                    |
| glaze             | Used for JSON loading                               | [MIT](https://github.com/stephenberry/glaze/blob/main/LICENSE)                                 |
| fmt               | Used for text formatting                            | [CUSTOM](https://github.com/fmtlib/fmt/blob/master/LICENSE)                                    |
| utfcpp            | Used for utf support                                | [Boost Software License 1.0](https://github.com/nemtrif/utfcpp/blob/master/LICENSE)            |
| imgui             | Used for ui rendering                               | [MIT](https://github.com/ocornut/imgui/blob/master/LICENSE.txt)                                |
| stb_image.h       | Used for loading images (png,gif,jpeg)              | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE)                     |
| stb_image_write.h | Used for writting images                            | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE)                     |
| stb_freetype.h    | Used for loading fonts                              | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE)                     |
| stb_easy_font.h   | Used for drawing debug text                         | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE)                     |
| thread-pool       | Used for managing async threading                   | [MIT](https://github.com/bshoshany/thread-pool/blob/master/LICENSE.txt)                        |
| qhull             | Used for calculating convex hulls from given points | [CUSTOM](https://github.com/qhull/qhull/blob/master/COPYING.txt)                               |
| fastgltf          | Used for loading gltf2.0 models                     | [MIT](https://github.com/spnda/fastgltf/blob/main/LICENSE.md)                                  |
| catch2            | Used for testing                                    | [Boost Software License 1.0](https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt)        |
| bass              | Used for loading sounds                             | [FREE / PAID](https://www.un4seen.com/bass.html)                                               |
| JoltPhysics       | Used for managing 3D physics                        | [MIT](https://github.com/jrouwe/JoltPhysics/blob/master/LICENSE)                               |
| muli              | Used for managing 2D physics                        | [MIT](https://github.com/Sopiro/Muli/blob/master/LICENSE.txt)                                  |
| webmlib & vpx     | Used for loading webm                               | [BSD 3-Clause](https://github.com/webmproject/libwebm/blob/main/LICENSE.TXT)                   |
| libwebp           | Used for WEBP loading                               | [BSD 3-Clause](https://github.com/webmproject/libwebp/blob/main/COPYING)                       |
| luabridge3        | Used for loading & managing lua scripting           | [MIT](https://github.com/kunitoki/LuaBridge3/blob/master/LICENSE.txt)                          |
| luau              | Used for lua scripting                              | [MIT](https://github.com/luau-lang/luau/blob/master/LICENSE.txt)                               |
| lua               | Used for lua scripting                              | [CUSTOM](https://github.com/luau-lang/luau/blob/master/lua_LICENSE.txt)                        |
| curl              | Used for HTTP / HTTPS requests                      | [MIT](https://github.com/libcpr/cpr/blob/master/LICENSE)                                       |
| libcpr            | Used for HTTP / HTTPS requests                      | [MIT](https://github.com/libcpr/cpr/blob/master/LICENSE)                                       |
| lunasvg           | Used for SVG loading                                | [MIT](https://github.com/sammycage/lunasvg/blob/master/LICENSE)                                |
| meshoptimizer     | Used to optimize meshes                             | [MIT](https://github.com/zeux/meshoptimizer/blob/master/LICENSE.md)                            |
| ozz-animation     | Used to animate skinned meshes                      | [MIT](https://github.com/guillaumeblanc/ozz-animation/blob/master/LICENSE.md)                  |
| spdlog            | Used for logging                                    | [MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE)                                      |
