![Build](https://github.com/ebu/ear-production-suite/workflows/Build/badge.svg)

# EAR production suite


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

### Dependencies

Required tools:
- Compiler with C++14 support
- [CMake](https://www.cmake.org) build systerm (version 3.8 or higher)

Required additional libraries:
- [JUCE](https://github.com/WeAreROLI/JUCE), tested with version 5.4.3
- [VST3 SDK](https://github.com/steinbergmedia/vst3sdk), tested with version 3.6.12
- [Boost](https://www.boost.org) (version 1.66 or higher)
- protobuf
- nng
- spdlog
- yaml-cpp

### Building from source

#### Get required SDKs

By default the required SDKs will be included from their location in submodules. If you would like to include them from elsewhere follow the steps below:

If not installed already, get the dependencies first, store them in some arbitrary directory,
in the following referenced by `$PATH_TO_SDKS`.

```
mkdir $PATH_TO_SDKS
git clone --branch vstsdk3612_03_12_2018_build_67 --recursive https://github.com/steinbergmedia/vst3sdk.git $PATH_TO_SDKS/VST3
git clone --branch 5.4.3 https://github.com/WeAreROLI/JUCE.git $PATH_TO_SDKS/JUCE
```

#### Install library dependencies

It is recommended to install the required dependencies using a package manager:
  - [Vcpkg](https://github.com/microsoft/vcpkg) on Windows: `vcpkg install protobuf nng spdlog yaml-cpp boost`
  - [Homebrew](https://brew.sh/) on OSX: `brew install protobuf nng spdlog yaml-cpp`

Please **note** that the *default and preferred way* on Windows is to link with the static windows runtime.
This means that you need to use a suitable `vcpkg` target triplet, e.g. `VCPKG_TARGET_TRIPLET=x64-windows-static` or `VCPKG_TARGET_TRIPLET=x86-windows-static`.

#### Build
```
mkdir build
cd build
cmake .. 
cmake --build .
```

If you would like to specify custom sdk locations, you can do so when configuring by passing the following parameters to cmake
```
cmake .. -DJUCE_ROOT_DIR=$PATH_TO_SDKS/JUCE -DVST3_ROOT_DIR=$PATH_TO_SDKS/VST3
```

Instead of specifying the paths to JUCE / VST3 SDKs on the command line, you
may also set the environment variables `JUCE_ROOT_DIR` and `VST3_ROOT_DIR` accordingly.

Depending on your build setup and toolchain, you may need to provide additional options, e.g. to configure and use `Vcpkg` etc. Please consult the documentation of those tools on how to integrate and them with `CMake`.

#### Usage

The build VST3 plugins can be found within the build folder: 

`build/ear-production-suite/VST3` For the ear production suite plugins
`build/reaper_adm_export/VST3` For the adm export plugin used with other plugin suites

##### OSX

To use the plugins, one can either copy the plugins to one of the standard plugin locations
 - `/Library/Audio/Plug-Ins/VST3`
 - `~/Library/Audio/Plug-Ins/VST3`

or one has to adjust the VST3 plugin search path of the respective audio workstation (DAW) so the plugins can be found.

To create **standalone** plugin bundles (i.e. plugins that can be used on non-development machines without installing the dependencies first), the cmake installation step must be run within the build folder:
```
cd build
cmake --build . --target install
```

##### Windows

To use the plugins, one can either copy the plugins to the system plugin location.
 - `C:\Program Files\Common Files\VST3` 

It is also possible to add the `build/VST3` folder  the VST3 plugin search path of the respective audio workstation (DAW) so the plugins can be found.

**Note**: It is **required** to amend the `PATH` environment variable so the library dependencies can be found.
That means that after building the plugins from source using `Vcpkg`, the following directories
 must be added:
- `build\ear-plugin-suite\VST3`
- `build\ear-plugin-suite\lib`

If the installation build step has been run, only the installation target directory must be added to the `PATH` variable.
