---
layout: page
title: EAR Production Suite
subtitle: A collection of production tools for Audio Definition Model (ADM) compliant production, brought to you by EBU, BBC R&D and IRT.
---

<div markdown="1" class="text_section">
The EAR Production Suite (EPS) is a set of VST® plugins for digital audio workstations (DAWs) that enable sound engineers to produce immersive and personalizable content using the [Audio Definition Model](https://www.itu.int/rec/R-REC-BS.2076) (ADM) format and to monitor it for any [ITU-R BS.2051](https://www.itu.int/rec/R-REC-BS.2051/en) loudspeaker configuration using the [ITU ADM Renderer](https://www.itu.int/rec/R-REC-BS.2127/en). ADM is the only format available for codec-agnostic [Next Generation Audio](https://tech.ebu.ch/nga) (NGA) productions. Moreover, the EAR Production Suite enables professionals to import and export ADM files, compliant to the [EBU ADM Production profile](https://tech.ebu.ch/docs/tech/tech3392.pdf). The VST® plugins are currently optimized for the Reaper DAW, which features an extension interface that is used to import and export ADM files within a BW64 container. The EAR Production Suite was designed to demonstrate the intended use of the ADM in audio production workflows, so that the standards can be adopted in other professional tools in a consistent manner.

The EAR Production Suite began as a joint [open-source development](https://github.com/ebu/ear-production-suite) of [BBC R&D](https://bbc.co.uk/rd) and IRT under the EBU. It continues to be maintained by BBC R&D.

</div>

<div class="features">
  <div markdown="1" class="text_section feature">
  <img src="{{ site.baseurl }}/images/codec-agnostic2.png">
  Codec-agnostic NGA productions
  </div>

  <div markdown="1" class="text_section feature">
  <img src="{{ site.baseurl }}/images/speaker2.png">
  Mix independently of loudspeaker setup
  </div>

  <div markdown="1" class="text_section feature">
  <img src="{{ site.baseurl }}/images/document.png">
  Native support for the Audio Definition Model
  </div>

  <div markdown="1" class="text_section feature">
  <img src="{{ site.baseurl }}/images/opensource.png">
  Free open source implementation
  </div>

</div>

<div style="clear: both;"></div>

<div markdown="1" class="text_section">
## Download and Install
The EAR Production Suite carries the <a data-source="license_url" data-type="href"><span data-source="license_type" data-type="innertext">GPL v3.0</span> license</a>.
Your download and use of the EAR Production Suite is subject to your agreement that the terms of <a data-source="license_url" data-type="href"><span data-source="license_type" data-type="innertext">GPL v3.0</span> (<span data-source="license_type_long" data-type="innertext">GNU General Public License v3.0</span>)</a> will govern such download and use.
<br /><br />
Download the latest EAR Production Suite release (<span data-source="version" data-type="innertext">Unknown</span> - <span data-source="version_date" data-type="innertext">Unknown</span>):
  <div class="button-grid">
    <a data-source="download_windows_url" data-type="href"><button class="c-btn">📦 Download Windows (x64)</button></a>
    <a data-source="download_macos_universal_url" data-type="href"><button class="c-btn">📦 Download macOS (x64/ARM64)</button></a>
  </div>
<br /><br />
<p>
The EAR Production Suite is designed for REAPER 64-bit <span data-source="min_reaper_ver" data-type="innertext">v6.11</span> or greater, on a 64-bit OS (macOS or Windows). For 128 channel support, REAPER v7.0 or greater is required. 
An experimental <a data-source="download_linux_url" data-type="href">Linux Build (x64)</a> is also available, but please be aware that this is very experimental at this time and may be buggy.
View the <a data-source="readme_url" data-type="href">ReadMe and Change Log</a> for this release. Old versions are available <a href="https://github.com/ebu/ear-production-suite/releases">here.</a>
</p>
<br />
  <details>
    <summary>Show installation instructions</summary>
      <a name="installation"></a>
      <div class="text_section">
        <p>The EAR Production Suite can be installed using the Setup application provided within the download package. This is the easiest method to install the EAR Production Suite. Alternatively, you can manually install the various components of the software. </p>
        <p>Please note that for the Linux build, there is currently no Setup application and the software must be installed manually.</p>
      </div>

      <hr />
      <h3 class="text_section">
        Installation via Setup Application
      </h3>
      <ol>
        <li>Install <a href="https://www.reaper.fm/download.php">REAPER</a></li>
        <li>Download the package appropriate for your operating system above, then;
          <br>- <b>macOS:</b> Mount the disk image and run the Setup application contained within.
          <br>- <b>Windows:</b> Unzip the package to a temporary location and run the Setup application from that location.
        </li>
        <li>Open REAPER and go to Options -> Preferences -> Plug-Ins -> VST and click Rescan</li>
        <li>You should see a new menu option <b>File -> Create Project from ADM file</b> now. If you don't see this option and you are using Windows, it might be necessary to download and install the <a href="https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads">Visual C++ 2015 redistributable</a> ("vc_redist.x64.exe") from Microsoft.
        </li>
      </ol>

      <hr />
      <h3 class="text_section">
        Manual Installation
      </h3>
      <ol>
        <li>Install <a href="https://www.reaper.fm/download.php">REAPER</a></li>
        <li>Copy / install the <b>VST plugins</b> into your common VST folder.
          <br>- <b>macOS:</b> ~/Library/Audio/Plug-Ins/VST3
          <br>- <b>Linux:</b> ~./vst3
          <br>- <b>Windows:</b> C:\Program&nbsp;Files\Common&nbsp;Files\VST3
        </li>
        <li>Open REAPER and go to Options -> Preferences -> Plug-Ins -> VST and click Rescan</li>
        <li>Copy / install REAPER ADM <b>Extension</b> into the REAPER plugins folder. Ensure you include the ADMPresets subdirectory.
          <br>- <b>macOS:</b> ~/Library/Application Support/REAPER/UserPlugins
          <br>- <b>Linux:</b> ~/.config/REAPER/
          <br>- <b>Windows:</b> C:\Users\(username)\AppData\Roaming\REAPER\UserPlugins
          <div style="margin-left: 2em; margin-right: 4em;">
          <i>Note: If you have a previous version of the REAPER Extension installed to C:\Program&nbsp;Files\REAPER&nbsp;(x64)\Plugins\reaper_adm.dll, then this should be deleted on installation of the latest version.</i>
          </div>
        </li>
        <li>Restart REAPER</li>
        <li>You should see a new menu option <b>File -> Create Project from ADM file</b> now. If you don't see this option and you are using Windows, it might be necessary to download and install the <a href="https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads">Visual C++ 2015 redistributable</a> ("vc_redist.x64.exe") from Microsoft.
        </li>
      </ol>
  </details>
</div>

<div markdown="1" class="text_section">
## Quickstart

<details>
  <summary>Import ADM Files</summary>
  <ol>
    <li>Select in the menu <b>File -> Create Project from ADM file -> Create from ADM using EAR</b></li>
    <li>Wait while all ADM elements are being created as tracks and automation curves along with metadata input plugins for each object or channel bed. There will be also tracks and plugins created for the Scene and the Monitoring.</li>
    <li>Disable "Master send" for the <b>Monitoring</b> track routing and add your hardware output there</li>
    <li>Enjoy :)</li>
  </ol>
</details>

<details>
  <summary>Start with session template</summary>
  <ol>
    <li>Open template in REAPER</li>
    <li>You will find a number of tracks with plugins for further usage
      <br>- Two object tracks
      <br>- One channel-based track
      <br>- One EAR Scene bus
      <br>- Two EAR Monitoring buses, one for Stereo monitoring and one for 5.1
    </li>
    <li>The Scene Plug-in has already two audio programmes, one called "English" and one "German"</li>
    <li>All metadata connections between the plugins and I/O routings are set. You can start by importing your audio files into the tracks.</li>
    <li>Switch between the different renderings by exclusive-soloing (CMD+Alt+Click (macOS) / Ctrl+Alt+Click (Win)) the monitoring tracks.</li>
  </ol>
</details>
</div>

<div markdown="1" class="text_section">
## Video Tutorial
  <div class="yt-iframe">
    <iframe src="https://www.youtube-nocookie.com/embed/u7P5mEFY76k" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
  </div>
  <i><b>Please note:</b> The tutorial video describes manual installation of the EAR Production Suite which is no longer necessary.
  Please refer to the updated <a href="#installation">installation instructions</a> above.</i>
</div>

<div markdown="1" class="text_section" style="display: none;">
## Discussion Community
A discussion community for users of the EPS is available on the <a href="https://www.bbc.co.uk/makerbox/tools/ear-production-suite">BBC's MakerBox platform.</a>
  <div class="button-grid-wide">
    <a href="https://www.bbc.co.uk/makerbox/tools/ear-production-suite"><button class="c-btn">Join in the discussion! 💬</button></a>
  </div>
</div>

<div markdown="1" class="text_section">
## Frequently Asked Questions (FAQ)
  <details>
    <summary>What version of REAPER is required to run the EAR Production Suite?</summary>
      <div class="text_section">
        <p>The current version of the EAR Production Suite requires REAPER 64-bit, version <span data-source="min_reaper_ver" data-type="innertext">v6.11</span> or greater. </p>
        <p>To make use of 128 channels, REAPER 64-bit v7.0 or greater is required. </p>
      </div>
  </details>
  <details>
    <summary>Can I import ADM BW64 files generated by third-party tools using the EAR Production Suite?</summary>
      <div class="text_section">
        <p>Yes, this should be supported. We are keen to ensure interoperability with third-party tools and so the EAR Production Suite is very tolerant in the ADM it will import. Should you have any issues importing an ADM BW64 file generated by third-party software, we would be keen to hear from you. You can either <a href="https://github.com/ebu/ear-production-suite/issues">post an issue in the GitHub repository</a> or <a href="mailto:ear-production-suite-admins@list.ebu.ch">email us</a>.</p>
      </div>
  </details>
  <details>
    <summary>Will my exported ADM BW64 files work with other tools, such as the Dolby Atmos Production Suite?</summary>
      <div class="text_section">
        <p>Currently the files are not compatible with the Dolby Atmos Production Suite.</p>
        <p>The cause of this problem is that each set of tools currently uses a different ADM profile. The EAR Production Suite uses the <a href="https://tech.ebu.ch/docs/tech/tech3392.pdf">EBU ADM Production profile</a>, whereas the Dolby Atmos tools use <a href="https://developer.dolby.com/technology/dolby-atmos/adm-atmos-profile/">Dolby’s ADM profile specification</a>. The EAR Production Suite provides a conversion from the Dolby profile on import, however it only exports to the EBU Production profile and the Dolby tools do not accept this as input. We hope to provide support for ADM profile conversion in future.</p>
        <p>More generally, ADM support has been implemented by different manufacturers and there are currently some interoperability issues. The EBU ADM Production profile is not yet widely supported, but we are working with the industry to ensure proper interoperability, using common profiles at the various stages from production through to emission.</p>
      </div>
  </details>
  <details>
    <summary>Which head-trackers are compatible with the Binaural Monitoring plug-in?</summary>
      <div class="text_section">
        <p>The Binaural Monitoring plug-in accepts listener orientation data over OSC and responds to messages used by several other popular spatial audio plug-in suites. 
        Therefore, if you have a head-tracker which functions with SPARTA/COMPASS, IEM, ambiX, HedRot, AudioLab SALTE, Mach1, or the 3D Tune-In Toolkit, then it should also function with the EPS Binaural Monitoring plug-in.
        Simply ensure that the "Enable OSC" toggle is on, and that the port number is configured to match that of your head-tracker.
        Note that the plug-in can not use the port if another plug-in is already using it, so please ensure no other plug-ins are present which use the same port.
        </p>
        <p style="margin-top: 20px">If you would like to construct your own low-cost head-tracker for use with the Binaural Monitoring plug-in, consider the <a href="https://github.com/trsonic/nvsonic-head-tracker">nvsonic Head-Tracker</a> for a compatible solution. It should operate with the Binaural Monitoring plug-in using any of the presets for the third-party spatial audio plug-ins listed above.
        </p>
        <p style="margin-top: 20px">We are keen to gather feedback on head-tracker compatibility. Should you encounter any issues using your headtracker with the EAR Production Suite, you can either <a href="https://github.com/ebu/ear-production-suite/issues">post an issue in the GitHub repository</a> or <a href="mailto:ear-production-suite-admins@list.ebu.ch">email us</a>.
        </p>
      </div>
  </details>
  <details>
    <summary>Can I use my own HRTF with the Binaural Monitoring plug-in?</summary>
      <div class="text_section">
        <p>Yes, although this is a feature for advanced users since it requires some pre-processing of the impulse responses. </p>
        <p>The Binaural Monitoring plug-in is based around <a href="https://tech.ebu.ch/publications/tech3396">BEAR (Binaural EBU ADM Renderer)</a> which uses it's own custom file format. There are various reasons for this explained in the guide linked below. Ultimately this requires a SOFA file to be converted using bespoke tool. The guide for this process is <a href="https://github.com/ebu/bear/blob/main/doc/ir_processing.md">available in the BEAR repository</a>. </p>
        <p>Once you have generated a TF file for BEAR, this should be placed alongside the EAR Binaural Monitoring plug-in. This will probably be `~/Library/Audio/Plug-ins/VST3/ear-production-suite` in MacOS, or `C:\Program Files\Common Files\VST3\ear-production-suite` in Windows. When the plug-in UI is then reopened, a drop-down box should appear at the top of the plug-in allowing you to select between HRTF sets.</p>
      </div>
  </details>
  <details>
    <summary>Projects created with very old versions of the EPS no longer load correctly. How do I fix this?</summary>
      <div class="text_section">
        <p>This occurs because it has been necessary to change the ID's of the plug-ins. 
        We have developed utilities to automatically convert your old REAPER projects to ensure they work correctly with the new plug-ins.
        There is both a command line utility and a GUI application included in the `tools` directory of the download package.
        <ul>
          <li>To use the GUI application, simply launch it and drag-and-drop your RPP files on to it.</li>
          <li>To use the command line utility, pass a path to an input RPP file and a path to an output RPP file as arguments.</li>
        </ul>
        If you are unsure whether it is necessary to upgrade your projects, it is usually quite apparent when you attempt to open the project in REAPER since it will appear that the plug-ins are not found even though they are installed and usable from within REAPER. In any case, it is perfectly OK to pass projects through the upgrade utilities regardless of whether they need it or not.
        </p>
      </div>
  </details>

</div>

<div markdown="1" class="text_section">
## Contact
You can contact the developers of the EAR Production Suite via [this mail](mailto:ear-production-suite-admins@list.ebu.ch). For feedback, feature requests and bug reports, we would appreciate if you submit an [Issue](https://github.com/ebu/ear-production-suite/issues) on our Github page. If you are considering adding ADM support to your own tools, we would be happy to discuss and advise.
</div>
