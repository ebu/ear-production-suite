![Build](https://github.com/ebu/ear-production-suite/workflows/Build/badge.svg)

# EAR Production Suite


## Introduction

A collection of VST3 audio plugins, and an extention to REAPER for production of next generation audio content using the EBU ADM Renderer.

## Download/Install prebuilt plugins

You can download prebuilt versions of the plugins and extension using the following links.

Releases: https://ear-production-suite.ebu.io/

After download copy the `ear-production-suite` folder and the `ADM Export Source` plugin to the correct folder as shown in the following table.

| System                    | Folder                                   |
| ------------------------- | ---------------------------------------- |
| macOS                     | ~/Library/Audio/Plug-ins/VST3            |
| Windows (64)              | C:\Program Files\Common Files\VST3       |

To install the extension you have to copy the dynamic library `reaper_adm` in the zip file to your REAPER resource path. 

You can open the folder from within REAPER via 

```
[Options] -> [Show REAPER resource path in explorer/finder...]
```

### OS X Gatekeeper
On macOS Catalina or above you may experience plugin load errors due to the new Gatekeeper feature.
You can disable Gatekeeper globally as per [this site](https://cronotek.net/blog/how-to-disable-gatekeeper-on-macos-mojave-and-catalina)
using this command:

```bash
sudo spctl --master-disable 
```

You can also manually validate the files after install with
```bash
sudo xattr -rd com.apple.quarantine \
~/Library/Application\ Support/REAPER/UserPlugins/reaper_adm.dylib \
/Library/Audio/Plug-Ins/VST3/ADM\ Export\ Source.vst3 \
/Library/Audio/Plug-Ins/VST3/ear-production-suite/
```
Substituting paths as needed if you have not installed to the default locations.

## Building from source

### Using vcpkg and cmake presets

The recommended way to build the plugins is via CMake's preset mechanism. Before you can make use of that you'll need a few tools.

- Compiler with C++14 support
- [CMake](https://www.cmake.org) build system (version 3.21.0 or higher for `--preset` support, 3.8 or higher for manual build)
- [Ninja](https://ninja-build.org/)

Note that 3.21.0 is a very recent revision of CMake, if you already have it installed you may need to upgrade.

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

