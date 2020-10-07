# EAR Production Suite (Beta release)

The latest builds are available under [Releases](https://github.com/ebu/ear-production-suite/releases).
Please download the files from there. The EAR Production Suite (EPS) download bundle comprises of multiple components:

- EPS VST® Plugins
- EPS REAPER Extension
- ADM Export Source VST® Plugin
- REAPER session template


## Installation

The EPS is designed for REAPER 64-bit, on a 64-bit OS (macOS or Windows)

1. Install REAPER from https://www.reaper.fm/download.php
2. Copy / install the **VST plugins** into your common VST folder. 
    - Windows: C:\Program Files\Common Files\VST3
    - macOS: ~/Library/Audio/Plug-Ins/VST3
3. Open REAPER and go to Options -> Preferences -> Plug-Ins -> VST and click Rescan
4. Copy / install REAPER ADM **Extension** into the REAPER plugins folder. Ensure you include the ADMPresets subdirectory.
    - Windows: C:\Program Files\REAPER (x64)\Plugins
    - macOS: ~/Library/Application Support/REAPER/UserPlugins 
5. Restart REAPER
6. You should see a new menu option **File -> Create Project from ADM file** now.
If you don't see this option and you are using Windows, it might be neccesary to download and install the [Visual C++ 2015 redistributable](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads) ("vc_redist.x64.exe") from Microsoft.

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
~/Library/Audio/Plug-Ins/VST3/ADM\ Export\ Source.vst3 \
~/Library/Audio/Plug-Ins/VST3/ear-production-suite/
```
Substituting paths as needed if you have not installed to the default locations.

## How to use (short version)
### Import ADM Files
1. Select in the menu **File -> Create Project from ADM file -> Create from ADM using EAR**
2. Wait while all ADM elements are being created as tracks and automation curves along with metadata input plugins for each object or channel bed. There will be also tracks and plugins created for the Scene and the Monitoring.
3. Disable "Master send" for the **Monitoring** track routing and add your hardware output there
3. Enjoy :)

### Create from scratch
1. Add an "EAR Object" Plugin for each audio object track or an "EAR DirectSpeakers" Plugin for a channel bed. Make sure to 
increase the track mapping parameter value +1 for each new audio object input plugin (or + the number of your channel bed). So the first audio object track in the REAPER session should be one, the next two, ...
2. Create a new track for the "EAR Scene" plugin and add it there.
3. Connect all audio objects tracks to the "EAR Scene" track as "Receives" in the I/O option. Make sure to choose the correct bus size (i.e. Mono).
4. Create a new track for the EAR Monitoring Plugin and add it there.
5. Connect the Scene track to the Monitoring track as "Receives" in the I/O option. Change the bus size to 64 (which is currently the limit for one Renderer plugin).
6. Add the hardware output for the Monitoring track in the I/O options.
7. Enter and author your ADM parameters in Object, DirectSpeakers and Scene Plugins. **Please note that the Scene Plugin is steering what you hear. So any items which are not added to a programme there, will not be rendered!**
8. Enjoy :)

### Start with session template
1. Open template in REAPER
2. You will find a number of tracks with plugins for further usage
    - Two object tracks
    - One channel-based track
    - One EAR Scene bus
    - Two EAR Monitoring buses, one for Stereo monitoring and one for 5.1
3. The Scene Plugin has already two audio programmes, one called "English" and one "German"
4. All metadata connections between the plugins and I/O routings are set. You can start by importing your audio files into the tracks.
5. Switch between the different renderings by exclusive-soloing (CMD+Alt+Click (MacOS) / Ctrl+Alt+Click (Win)) the monitoring tracks.

## How to use (long version)
Have a look at this video where these points are explained:
- Introduction
- Installation
- Tutorial 1: Understanding the EAR Production Suite Plugins
- Tutorial 2: Creating an ADM Project from scratch with the EAR Production Suite Plugins
- Tutorial 3: Using an Existing ADM File with the EAR Production Suite Plugins
- Tutorial 4: Using Third-Party Spatial Audio Plugins with the ADM Export Source Plugin
- Tutorial 5: Using Non-Object Audio with the ADM Export Source Plugin

[![Watch the video](https://irt-a.akamaihd.net/EAR-Production-Suite/Intro_image.png)](https://irt-a.akamaihd.net/EAR-Production-Suite/Intro_Beta-release.mp4)

## Survey
We would highly appreciate if you could fill in [this survey](https://docs.google.com/forms/d/e/1FAIpQLSfosvonWSOUt_-Q30MJtcGERs2XVqS2EaKauOzGsc3B4oWKIA/viewform). It takes only a few minutes and will help us to create a better tool based on your feedback.

## What's still missing
You are testing a beta version of the EAR Production Suite and some features are still under development or missing in this version:

- No support for HOA, Binaural and Matrix typeDefinition.
- Only currently supporting a subset of DirectSpeakers common definitions.
- The Object plugin panner is operating in egocentric mode (aka VBAP), using polar coordinate. We have not yet decided whether we will support also allocentric panning (using cartesian coordinates) in future. However, ADM files using allocentric panning / cartesian coordinates are converted during the import process. 
- No support for "nested" Objects - on import, ADM Programmes are currently flattened to a single tier of Objects.
- Only one instance of EAR Scene should run on a machine at any time.
- Re-opening and exporting from a saved project using the FB360 plugins may produce incorrect ADM. This is due to a bug in the FB360 plugins which has been reported. 
- ...

## Issues
Before submitting a new report in [Issues](https://github.com/ebu/ear-production-suite/issues), please check if your problem or feature request is already known. 
