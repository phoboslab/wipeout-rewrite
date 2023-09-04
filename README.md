# wipEout Rewrite

This is a re-implementation of the 1995 PSX game wipEout.

Play here: https://phoboslab.org/wipegame/

More info in my blog: https://phoboslab.org/log/2023/08/rewriting-wipeout

⚠️ Work in progress. Expect bugs.

# Building

The Wipeout rewrite supports two different platform-backends:
[SDL2](https://github.com/libsdl-org/SDL) and
[Sokol](https://github.com/floooh/sokol).
The only difference in features is that the SDL2 backend supports game
controllers (joysticks, gamepads), while the Sokol backend does not.
The Sokol backend is also only supported on macOS, Linux, Windows and Emscripten.

For Linux & Unix-likes a simple Makefile is a provided. Additionally, this
project can be build with [CMake](https://cmake.org) for all platforms.

Consult the following sections for how to install the prerequisites for your platform:


## Linux & Unix-like

Building on Linux should be as simple as installing CMake, GLEW, and the
necessary platform libraries from your package manager.
For brevity, this guide assumes that the necessary development tools (i.e. a C
compiler, make) have already been installed.
The SDL2 platform should only require the `sdl2` library and headers, whilst the
Sokol platform requires the library/headers for:

- `X11`
- `Xi`
- `Xcursor`
- `ALSA`

The following snippets list the specific package manager invocations for
popluar \*nix OSs:

### Debian/Ubuntu

```sh
apt install cmake libglew-dev
# For SDL2
apt install libsdl2-dev
# For Sokol
apt install libx11-dev libxcursor-dev libxi-dev libasound2-dev
```

### Fedora

```sh
dnf install cmake glew-devel
# For SDL2
dnf install SDL2-devel
# For Sokol
dnf install libx11-devel libxcursor-devel libxi-devel alsa-lib-devel
```

### Arch Linux

```sh
pacman -S cmake glew
# For SDL2
pacman -S sdl2
# For Sokol
pacman install libx11 libxcursor libxi alsa-lib
```

### OpenSUSE

```sh
zypper install cmake glew-devel
# For SDL2
zypper install SDL2-devel
# For Sokol
zypper install libx11-devel libxcursor-devel libxi-devel alsa-lib-devel
```

### FreeBSD

```sh
pkg install cmake sdl2
```

### OpenBSD

```sh
pkg_add cmake sdl2
```

Note that the Sokol platform is not supported on the BSDs, since the Sokol
headers themselves do not support these Operating Systems.

With the packages installed, you can now setup and build:

```sh
# With make for SDL2 backend
make sdl

# With make for Sokol backend
make sokol

# With cmake
cmake -S path/to/wipeout-rewrite -B path/to/build-dir
cmake --build path/to/build-dir
```

## macOS

Currently only the SDL2 platform works.
macOS is very picky about the GLSL shader version when compiling with Sokol and
OpenGL3.3; it shouldn't be too difficult to get it working, but will probably
require a bunch of `#ifdefs` for SDL and WASM.
Pull-requests welcome!

It is recommended to use [Homebrew](https://brew.sh) to fetch the required
software, other solutions (e.g. MacPorts) may work but have not been tested.
Using homebrew, you can install the required software with the following:

```sh
brew install cmake
# For SDL2
brew install sdl2
# Nothing extra needed for Sokol
```

With the packages installed, you can now setup and build:

```sh
cmake -S path/to/wipeout-rewrite -B path/to/build-dir \
	-DCMAKE_PREFIX_PATH="$(brew --prefix sdl2)"
cmake --build path/to/build-dir
```

## Windows

### clang-cl

Building natively on Windows requires a more complicated setup. The source code
relies on GCC extensions that are not supported by `msvc`, which requires the
use of `clang-cl`.
The simplest way to get a build environment with `clang-cl` is to download and
install [Visual Studio](https://visualstudio.microsoft.com/downloads/) (2022 at
the time of writing) with the "Desktop development with C++" option selected.
Also make sure to select "Clang C++ compiler for Windows" in "Individual
Components" if it hasn't been already.

The next step is to acquire development versions of SDL2 and GLEW.
The easiest way is to install [vcpkg](https://vcpkg.io) and let Visual Studio's
integration build and install it for you.
Follow the [vcpkg "Getting Started" guide](https://vcpkg.io/en/getting-started)
and integrate it with Visual Studio.

Finally, open Visual Studio, select "Open a local folder", and navigate to the
directory where you have cloned this repo.
Visual Studio should automatically configure itself to build with CMake, and
build the necessary libraries using vcpkg.
Since this repository contains a `CMakeSettings.json` file, there should already
be CMake configurations listed in the menubar dropdown.
When adding a new configuration, make sure to use the `clang_cl` toolsets.
Select the config you want from the list and build using `F7`, the build
artifacts should be under `path\to\wipeout-rewrite\build`.

### MSYS2

Building with [MSYS2](https://www.msys2.org/) is sightly easier but still
involves a bit of configuration.
Download and install MSYS2 using the installer, and enter a MSYS2 environment
using the start menu. For this guide we're using the `UCRT` environment, but the
others work just as well.

Install the following packages using `pacman`:

```sh
pacman -S mingw-w64-ucrt-x86_64-{toolchain,cmake,SDL2,glew}
```

With the packages installed, you can now setup and build:

```sh
# With make for SDL2 backend
make sdl

# With make for Sokol backend
make sokol

# With cmake
cmake -S path/to/wipeout-rewrite -B path/to/build-dir
cmake --build path/to/build-dir
```

## Emscripten

Download and install the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html),
so that `emcc` and `emcmake` is in your path.
Linux users may find it easier to install using their distro's package manager
if it is available.
Note that only the Sokol platform will work for WebAssembly builds.

With the SDK installed, you can now setup and build:

```sh
# With make (combined full and minimal builds)
make wasm

# With cmame (full or minimal builds specified via -DMINIMAL_BUNDLE={OFF|ON})
emcmake cmake -S path/to/wipeout-rewrite -B path/to/build-dir -DPLATFORM=SOKOL
cmake --build path/to/build-dir
```

## Build Flags for cmake

The following is a table for project specific build flags using CMake:

| Flag             | Description                                                     | Options                                                                              | Default                                                                                     |
|------------------|-----------------------------------------------------------------|--------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------|
| `PLATFORM`       | The platform to build for.                                      | `SDL2`, `SOKOL`                                                                      | `SDL2`                                                                                      |
| `RENDERER`       | Graphics renderer.                                              | `GL` for OpenGL 3, `GLES2` for OpenGL ES 2, `SOFTWARE` for a pure software renderer. | `GL`                                                                                        |
| `USE_GLVND`      | Link against the OpenGL Vendor Neutral Dispatch libraries.      | `ON`, `OFF`                                                                          | `ON`, falling back to `OFF` if the libraries aren't found or an OpenGL renderer isn't used. |
| `MINIMAL_BUNDLE` | Do not include the music/intro video when building for the web. | `ON`, `OFF`                                                                          | `OFF`                                                                                       |


# Running

This repository does not contain the assets (textures, 3d models etc.) required to run the game. This code mostly assumes to have the PSX NTSC data, but some menu models from the PC version are required as well. Both of these can be easily found on archive.org and similar sites. The music (optional) needs to be provided in [QOA format](https://github.com/phoboslab/qoa). The intro video as MPEG1.

The directory structure is assumed to be as follows

```
./wipegame # the executable
./wipeout/textures/
./wipeout/music/track01.qoa
./wipeout/music/track02.qoa
...
```

Note that the blog post announcing this project may or may not provide a link to a ZIP containing all files needed. Who knows!

Optionally, if you want to use a game controller that may not be supported by SDL directly, you can place the [gamecontrollerdb.txt](https://github.com/gabomdq/SDL_GameControllerDB) in the root directory of this project (along the compiled `wipegame`).



# Ideas for improvements

PRs Welcome.

## Not yet implemented

Some things from the original game are not yet implemented in this rewrite. This includes

- screen shake effect
- game-end animations, formerly `Spline.cpp` (the end messages are just shown over the attract mode cameras)
- viewing highscores in options menu
- reverb for sfx and music when there's more than 4 track faces (tunnels and such)
- some more? grep the source for `TODO` and `FIXME`

## Gameplay, Visuals

- less punishing physics for ship vs. ship collisions
- less punishing physics for sideways ship vs. track collisions (i.e. wall grinding like in newer wipEouts)
- somehow resolve the issue of inevitably running into an enemy that you just shot
- add option to lessen the roll in the internal view
- add additional external view that behaves more like in modern racing games
- dynamic lighting on ships
- the scene geometry could use some touch-ups to make an infinite draw distance option less awkward
- increase FOV when going over a boost
- better menu models for game exit and video options
- gamepad analog input feels like balancing an egg
- fix collision issues on junctions (also present in the original)

## Technical

- implement frustum culling for scene geometry, the track and ships. Currently everything within the fadeout radius is drawn.
- put all static geometry into a GPU-side buffer. Currently all triangles are constructed at draw time. Uploading geometry is complicated a bit by the fact that some scene animations and the ship's exhaust need to update geometry for each frame.
- the menu system is... not great. It's better than the 5000 lines of spaghetti that it was before, but the different layouts need a lot of `if`s
- the save data is just dumping the whole struct on disk. A textual format would be preferable.
- since this whole thing is relying on some custom assembled assets anyway, maybe all SFX should be in QOA format too (like the music). Or switch everything to Vorbis.
- a lot of functions assume that there's just one player. This needs to be fixed for a potential splitscreen mode.


# License

There is none. This code may or may not be based on the source code of the PC (ATI-Rage) version that was leaked in 2022. If it were, it would probably violate copyright law, but it may also fall under fair use ¯\\\_(ツ)\_/¯

Working with this source code is probably fine, considering that this game was originally released 28 years ago (in 1995), that the current copyright holders historically didn't care about any wipEout related files or code being available on the net and that the game is currently not purchasable in any shape or form.

In any case, you may NOT use this source code in a commercial release. A commercial release includes hosting it on a website that shows any forms of advertising.

PS.: Hey Sony! If you're reading this, I would love to work on a proper, officially sanctioned remaster. Please get in touch <3
