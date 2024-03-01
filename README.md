![Build](https://github.com/ebu/ear-production-suite/workflows/Build/badge.svg)

# EAR Production Suite


## Introduction

A collection of VST3 audio plugins, and an extention to REAPER for production of next generation audio content using the EBU ADM Renderer.

## Download/Install prebuilt plugins

You can download prebuilt versions of the plugins and extension using the link below. There is also an experimental Linux build available, but be aware this is only experimental and could prove buggy.

Releases: https://ear-production-suite.ebu.io/

There are two methods of installation; using the setup application, or manual.

### Setup Application

For Windows and MacOS, the download package will contain a Setup application.

For MacOS, mount the downloaded disk image and run the "Setup EAR Production Suite" application within.

For Windows, extract the contents of the downloaded package to a temporary location and run the setup application.

### Manual Install

Please refer to `README.pdf`, which is available in the root of the Release packages or within the `packaging` subdirectory of this repository.

## Building from source

### Using vcpkg and cmake presets

The recommended way to build the plugins is via CMake's preset mechanism. Before you can make use of that you'll need a few tools.

- Compiler with C++14 support
- [CMake](https://www.cmake.org) build system (version 3.21.0 or higher for `--preset` support, 3.8 or higher for manual build)
- [Ninja](https://ninja-build.org/)

### MacOS
##### Build environment
The easiest way to set up a suitable environment is to follow the [homebrew](https://brew.sh/) setup instructions. Once you have a working homebrew:
```shell
brew update
brew install cmake ninja
```
The following instructions are for an **x64 (Intel processor)** build. For **ARM64 (Apple Silicon)**, use the `macos-default-arm64` preset in place of `macos-default`.

#### Building
```shell
git clone --recursive https://github.com/ebu/ear-production-suite.git
cd ear-production-suite
./submodules/vcpkg/bootstrap-vcpkg.sh # ensures vcpkg is setup 
cmake --preset macos-default          # configures project, downloads & builds dependencies
cmake --build --preset macos-default  # builds project
```

#### Installing
```shell
cmake --build --preset macos-default --target install
```

The location of the installed VST3 plugins will be 
```shell
~/Library/Audio/Plug-Ins/VST3/
```
The location of the installed REAPER extension will be
```shell
~/Library/Application Support/REAPER/UserPlugins/
```

### Windows
[Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/vs/), installed with C++ support provides a suitable compiler. 

You'll need to install [CMake](https://www.cmake.org) and [Ninja](https://ninja-build.org/) manually, add their installation locations to your PATH. Then, execute the following steps from a Visual Studio x64 developer command prompt.

#### Building

```bash
git clone --recursive https://github.com/ebu/ear-production-suite.git
cd ear-production-suite
.\submodules\vcpkg\bootstrap-vcpkg.bat # ensures vcpkg is setup
cmake --preset windows-default         # configures project, downloads & builds dependencies
cmake --build --preset windows-default # builds project
```
#### Installing
From an administrator command prompt, run
```bash
cmake --build --preset windows-default --target install
```

The VST3 plugins will be installed to your Windows Program Files directory.
The location of the installed VST3 plugins will be
```shell
<Program Files>\Common Files\VST3\
```
The REAPER extension will be installed to the AppData directory for the current user (as specified by the `AppData` environment variable, which will normally resolve to the `Roaming` subdirectory.)
The location of the installed REAPER extension will be
```shell
~\AppData\Roaming\REAPER\UserPlugins\
```

### Linux

These instructions are for Ubuntu 20.04, but other distributions should be similar.

Presets are currently only defined for x86_64; other architectures are untested but can probably be built manually or with a modified CMakePresets.json file.

##### Build environment

To get an up-to-date version of cmake, follow the instructions on the [Kitware APT repository](https://apt.kitware.com/).

For the rest of the dependencies, run:

```shell
# tools for vcpkg
sudo apt-get install build-essential curl zip unzip tar git cmake ninja-build pkg-config
# graphics libraries
sudo apt-get install libx11-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libglu1-mesa-dev libfreetype6-dev
```

#### Building
```shell
git clone --recursive https://github.com/ebu/ear-production-suite.git
cd ear-production-suite
./submodules/vcpkg/bootstrap-vcpkg.sh     # ensures vcpkg is set up
cmake --preset linux-default-x64          # configures project, downloads & builds dependencies
cmake --build --preset linux-default-x64  # builds project
```

#### Installing
```shell
cmake --build --preset linux-default-x64 --target install
```

The location of the installed VST3 plugins will be
```shell
~/.vst3
```
The location of the installed REAPER extension will be
```shell
~/.config/REAPER/UserPlugins/
```

### Customising installation location

If the defaults do not suit you, they can be customised with two CMake variables

```shell
EPS_PLUGIN_INSTALL_PREFIX  # This specifies the directory to which the plugins should be installed
````
```shell
EPS_EXTENSION_INSTALL_PREFIX  # This specifies the directory containing the REAPER plugins subdirectory
                              # to which the extension should be installed
```
If manually set, both of these variables must end with a trailing `/`.

They should be provided at configure stage using CMake's `-D` flag (set cache variable). For example:

```shell
cmake --preset macos-default -DEPS_PLUGIN_INSTALL_PREFIX="/a/hidden/place/" -DEPS_EXTENSION_INSTALL_PREFIX="/top/secret/location/"
```

