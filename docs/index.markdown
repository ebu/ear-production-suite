---
layout: page
title: EAR Production Suite
subtitle: A collection of production tools for Audio Definition Model (ADM) compliant production, brought to you by EBU, BBC R&D and IRT.
---

<div markdown="1" class="text_section">
The EAR Production Suite (EPS) is a set of VST® plugins for digital audio workstations (DAWs) that enable sound engineers to produce immersive and personalizable content using the [Audio Definition Model](https://www.itu.int/rec/R-REC-BS.2076) (ADM) format and to monitor it for any [ITU-R BS.2051](https://www.itu.int/rec/R-REC-BS.2051/en) loudspeaker configuration using the [ITU ADM Renderer](https://www.itu.int/rec/R-REC-BS.2127/en). ADM is the only format available for codec-agnostic [Next Generation Audio](https://tech.ebu.ch/nga) (NGA) productions. Moreover, the EAR Production Suite enables professionals to import and export ADM files, compliant to the [EBU ADM Production profile](https://tech.ebu.ch/docs/tech/tech3392.pdf). The VST® plugins are currently optimized for the Reaper DAW, which features an extension interface that is used to import and export ADM files within a BW64 container. The EAR Production Suite was designed to demonstrate the intended use of the ADM in audio production workflows, so that the standards can be adopted in other professional tools in a consistent manner.

The EAR Production Suite is a joint [open-source development](https://github.com/ebu/ear-production-suite) of [BBC R&D](https://bbc.co.uk/rd) and [IRT](https://www.irt.de/en/home)
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
## Video Tutorial
  <div class="yt-iframe">
    <iframe src="https://www.youtube-nocookie.com/embed/u7P5mEFY76k" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
  </div>
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
    <li>The Scene Plugin has already two audio programmes, one called "English" and one "German"</li>
    <li>All metadata connections between the plugins and I/O routings are set. You can start by importing your audio files into the tracks.</li>
    <li>Switch between the different renderings by exclusive-soloing (CMD+Alt+Click (macOS) / Ctrl+Alt+Click (Win)) the monitoring tracks.</li>
  </ol>
</details>
</div>

<div markdown="1" class="text_section">
## Installation
The EPS is designed for REAPER 64-bit, on a 64-bit OS (macOS or Windows)
  <details>
    <summary>Show installation instructions</summary>
      <div markdown="1" class="text_section">
        <i><b>Please Note:</b> The EPS is currently incompatible with Language Packs. REAPER may fail to load when using the EPS with a language pack installed. Support will be provided in a future release.</i>
      </div>
      <ol>
        <li>Install <a href="https://www.reaper.fm/download.php">REAPER</a></li>
        <li>Copy / install the <b>VST plugins</b> into your common VST folder.
          <br>- Windows: C:\Program Files\Common Files\VST3
          <br>- macOS: ~/Library/Audio/Plug-Ins/VST3
        </li>
        <li>Open REAPER and go to Options -> Preferences -> Plug-Ins -> VST and click Rescan</li>
        <li>Copy / install REAPER ADM <b>Extension</b> into the REAPER plugins folder. Ensure you include the ADMPresets subdirectory.
          <br>- Windows: C:\Program Files\REAPER (x64)\Plugins
          <br>- macOS: ~/Library/Application Support/REAPER/UserPlugins
        </li>
        <li>Restart REAPER</li>
        <li>You should see a new menu option <b>File -> Create Project from ADM file</b> now. If you don't see this option and you are using Windows, it might be neccesary to download and install the <a href="https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads">Visual C++ 2015 redistributable</a> ("vc_redist.x64.exe") from Microsoft.
        </li>
      </ol>
  </details>

  <div class="button-grid">
    <a href="https://github.com/ebu/ear-production-suite/releases/download/v0.6.0-EPS-beta/EPS-macOS.v0.6.0.zip"><button class="c-btn">Download macOS</button></a>
    <a href="https://github.com/ebu/ear-production-suite/releases/download/v0.6.0-EPS-beta/EPS-Windows.v0.6.0.zip"><button class="c-btn">Download Windows</button></a>
  </div>
</div>

<div markdown="1" class="text_section">
## Contact
You can contact the developers of the EAR Production Suite via [this mail](mailto:ear-production-suite-admins@list.ebu.ch). For feedback, feature request and bug reports, we would appreciate if you submit an [Issue](https://github.com/ebu/ear-production-suite/issues) on our Github page. If you are considering adding ADM support to your own tools, we would be happy to discuss and advise.
</div>
