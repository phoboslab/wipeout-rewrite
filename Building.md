The Wipeout rewrite supports two different platform-backends:
[SDL2](https://github.com/libsdl-org/SDL) and
[Sokol](https://github.com/floooh/sokol).
The only difference in features is that the SDL2 backend supports game
controllers (joysticks, gamepads), while the Sokol backend does not.
The Sokol backend is also only supported on macOS, Linux, Windows and Emscripten.

# Building For Your Platform

For Linux & Unix-likes a simple Makefile is a provided. Additionally, this
project can be build with [CMake](https://cmake.org) for all platforms.

Consult the following sections for how to install the prerequisites for your platform:


## Linux & Unix-like

Building on Linux should be as simple as installing CMake, GLEW, and the
necessary platform libraries from your package manager.
For brevity, this guide assumes that the necessary development tools (i.e. a C
complier, make) have already been installed.
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
cd wipeout-rewrite && make wasm

# With cmame (full or minimal builds specified via -DMINIMAL_BUNDLE={OFF|ON})
emcmake cmake -S path/to/wipeout-rewrite -B path/to/build-dir -DPLATFORM=SOKOL
cmake --build path/to/build-dir
```

## Build Flags for cmake

The following is a table for project specific build flags using CMake:

| Flag             | Description                                                     | Options                                                                              | Default                                                                                     |
|------------------|-----------------------------------------------------------------|--------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------|
| `PLATFORM`       | The platform to build for.                                      | `SDL2`, `SOKOL`                                                                      | `SDL2`                                                                                      |
| `RENDERER`       | Graphics renderer.                                              | `GL` for OpenGL 3, `GLES2` for OpenGL ES 2, `Software` for a pure software renderer. | `GL`                                                                                        |
| `USE_GLVND`      | Link against the OpenGL Vendor Neutral Dispatch libraries.      | `ON`, `OFF`                                                                          | `ON`, falling back to `OFF` if the libraries aren't found or an OpenGL renderer isn't used. |
| `MINIMAL_BUNDLE` | Do not include the music/intro video when building for the web. | `ON`, `OFF`                                                                          | `OFF`                                                                                       |
