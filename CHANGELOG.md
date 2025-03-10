UNRELEASED
Note: This release bumps the minimum required REAPER version from v6.11 to v6.37

* Fix occasional crash bug in start up of Binaural Monitoring plugin [#277](https://github.com/ebu/ear-production-suite/issues/277) [#278](https://github.com/ebu/ear-production-suite/pull/278)
* Fix fail to render when non-VST plugins are in FX chains [#279](https://github.com/ebu/ear-production-suite/issues/279) [#280](https://github.com/ebu/ear-production-suite/pull/280)
* Update render dialog code to support latest version of REAPER (v7.34) [#296](https://github.com/ebu/ear-production-suite/issues/296) [#297](https://github.com/ebu/ear-production-suite/pull/297)

Version 1.1.0b

* Fix installation on MacOS where Gatekeeper enforces translocation [#273](https://github.com/ebu/ear-production-suite/pull/273)

Version 1.1.0

* Support for 128 channels when using REAPER >=v7.0 [#244](https://github.com/ebu/ear-production-suite/issues/244) [#254](https://github.com/ebu/ear-production-suite/pull/254) [#267](https://github.com/ebu/ear-production-suite/pull/267)
* Improve import support from third-party tools [#259](https://github.com/ebu/ear-production-suite/pull/259)
* Support HRTF switching in Binaural Monitoring plugin [#266](https://github.com/ebu/ear-production-suite/pull/266)
* Fix monitoring plugins not properly supporting shared inputs [#257](https://github.com/ebu/ear-production-suite/issues/257) [#258](https://github.com/ebu/ear-production-suite/pull/258)
* Apply "version" attribute to ADM as required by ITU-R BS.2076-2 [#248](https://github.com/ebu/ear-production-suite/issues/248) [#255](https://github.com/ebu/ear-production-suite/pull/255)
* Performance fix for envelope creation [#252](https://github.com/ebu/ear-production-suite/pull/252) 
* Update BW64 lib for performance fixes [#260](https://github.com/ebu/ear-production-suite/issues/260)
* Allow user to opt-out of update-checking during setup [#256](https://github.com/ebu/ear-production-suite/pull/256)
* Warn user of impending FB360 and VISR support deprecation since these plugin suites are no longer maintained [#265](https://github.com/ebu/ear-production-suite/pull/265)

Version 1.0.0

* Setup Application included
* Support for ADM "Importance" parameter
* Fix bug in render dialog when using language packs [#215](https://github.com/ebu/ear-production-suite/issues/215)
* Experimental Linux build [#222](https://github.com/ebu/ear-production-suite/pull/222)
* Silence unused monitoring output channels (prevents pass-through of object audio) [#224](https://github.com/ebu/ear-production-suite/issues/224)
* Fix alignment of monitoring meters [#206](https://github.com/ebu/ear-production-suite/issues/206)
* Fix blank audioProgrammeLanguage on export [#213](https://github.com/ebu/ear-production-suite/issues/213)
* Export uses 2076-2 structures (omitting AudioTrackFormat and AudioStreamFormat)
* MacOS build fixes [#220](https://github.com/ebu/ear-production-suite/issues/220) [#221](https://github.com/ebu/ear-production-suite/issues/221)
* Include additional project templates
* Fix bus config when Input plug-ins on wide track [#228](https://github.com/ebu/ear-production-suite/issues/228)
* Fix Size param not updating [#229](https://github.com/ebu/ear-production-suite/issues/229)
* Fix plugin crash on other DAWs [#232](https://github.com/ebu/ear-production-suite/issues/232)
* Support plugin renaming in REAPER [#240](https://github.com/ebu/ear-production-suite/issues/240)
* Fix blank Scene when importing ADM with no high-level metadata [#242](https://github.com/ebu/ear-production-suite/issues/242)
* Fix parameters not updating in response to other parameter changes after JUCE update
* Fix render dialog controls inadvertently re-enabling
* Support render dialog changes in newer REAPER versions
* Wider REAPER version support (back to 6.11)
* Various performance improvements
* Use more appropriate bus layouts

Version 0.8.0

* Support custom object names rather than using track names [#214](https://github.com/ebu/ear-production-suite/pull/214)
* Support sharing of audio assets between objects (avoids asset duplication) [#223](https://github.com/ebu/ear-production-suite/pull/223) [#211](https://github.com/ebu/ear-production-suite/issues/211) [#207](https://github.com/ebu/ear-production-suite/issues/207)
* Import process finds and imports orphaned AudioObjects [#208](https://github.com/ebu/ear-production-suite/issues/208)
* Fix envelope point creation for blocks using jump position with rtime of 0 
* Fix crash when adding DirectSpeakers objects to programmes before their speaker layout is set [#209](https://github.com/ebu/ear-production-suite/issues/209)
* Fix rendering crash when using HOA without format set
* Fix rendering crash when using ADM Export Source plugin with unsupported asset types for plugin suite
* Update libadm to 0.14.0
* Fix import of some 2076-2 ADM structures
* Fix import of files with large individual audio assets
* Fix Read-only plugin parameters after JUCE upgrade

Version 0.7.3

* Fix 'no name' items in Scene and associated issues through major refactor [#197](https://github.com/ebu/ear-production-suite/issues/197)
* Support new render dialog controls in recent version of REAPER [#202](https://github.com/ebu/ear-production-suite/issues/202)
* Address silent crash logged on Windows when closing REAPER [#202](https://github.com/ebu/ear-production-suite/issues/202)
* Fix hang on MacOS with multiple export sources on render [#202](https://github.com/ebu/ear-production-suite/issues/202)
* Fix sockets left open after render [#202](https://github.com/ebu/ear-production-suite/issues/202)
* Fix Scene view orientation mismatch [#197](https://github.com/ebu/ear-production-suite/issues/197)
* Improve Scene GUI performance [#197](https://github.com/ebu/ear-production-suite/issues/197)
* Correct textbox alignment after JUCE upgrade [#197](https://github.com/ebu/ear-production-suite/issues/197)

Version 0.7.2

* Binaural monitoring axis inversion controls - closes [#177](https://github.com/ebu/ear-production-suite/issues/177)
* Save binaural monitoring config to file - closes [#182](https://github.com/ebu/ear-production-suite/issues/182)
* Patch JUCE so Windows can detect failure to open UDP port - closes [#169](https://github.com/ebu/ear-production-suite/issues/169)
* Don't crash if binaural monitoring plugin receives 0.6.0 metadata - closes [#180](https://github.com/ebu/ear-production-suite/issues/180)
* Fix potential race condition in weak pointer usage [#181](https://github.com/ebu/ear-production-suite/issues/181)
* Use mono mix for export waveform display, hide channel selection in export dialog [#185](https://github.com/ebu/ear-production-suite/pull/185)
* Disable renderers when exporting to ADM for improved export speed [#185](https://github.com/ebu/ear-production-suite/pull/185)
* Fix resolution of overlapping routings from HOA input plugin [#191](https://github.com/ebu/ear-production-suite/issues/191)
* Pushed custom libadm changes upstream and updated submodule [#189](https://github.com/ebu/ear-production-suite/pull/189)

Version 0.7.1 

* Updated JUCE framework to 6.1.5 [#171](https://github.com/ebu/ear-production-suite/pull/171), fixes arm64/macos crash on unhandled keydown events [#174](https://github.com/ebu/ear-production-suite/issues/174)
* Disabled interaction panel for non-object types in Scene [#173](https://github.com/ebu/ear-production-suite/pull/173)
* Prevent potential out-of-bounds array access on removal of right-most programme in scene [#175](https://github.com/ebu/ear-production-suite/pull/175)
* Fix crash when pressing backspace on HOA combobox [#172](https://github.com/ebu/ear-production-suite/pull/172)

Version 0.7.0

* Added Binaural Monitoring and HOA Input plugins [#156](http://github.com/ebu/ear-production-suite/pull/156)
* Improved default install target locations [#4](http://github.com/ebu/ear-production-suite/pull/4) [#79](http://github.com/ebu/ear-production-suite/pull/79) [#95](http://github.com/ebu/ear-production-suite/issues/95)
* Fixed bug that made adding items after moving a programme fail. [#5](http://github.com/ebu/ear-production-suite/pull/5)
* Added unique plugin uids and FX category for better DAW compatibility [#10](http://github.com/ebu/ear-production-suite/pull/10)
* Added REAPER project upgrade tool [#11](http://github.com/ebu/ear-production-suite/pull/11) [#136](http://github.com/ebu/ear-production-suite/pull/136)
* Properly persist parameters when saving/restoring a session [#17](http://github.com/ebu/ear-production-suite/pull/17)
* Fixed bug where bypassed parameters were exported to ADM [#52](http://github.com/ebu/ear-production-suite/pull/52)
* Improved support for building via Xcode project [#53](http://github.com/ebu/ear-production-suite/pull/53) [#66](http://github.com/ebu/ear-production-suite/pull/66)
* Fixed crash on exit in Debug mode [#54](http://github.com/ebu/ear-production-suite/pull/54)
* Fixed bug where exported block boundaries were not always contiguous [#55](http://github.com/ebu/ear-production-suite/pull/55)
* UI Improvements [#56](http://github.com/ebu/ear-production-suite/pull/56) [#114](http://github.com/ebu/ear-production-suite/pull/114)
* Fixed parameter update data race [#57](http://github.com/ebu/ear-production-suite/pull/57)
* Export will now set jumpPosition flag where appropriate [#62](http://github.com/ebu/ear-production-suite/pull/62)
* Moved some operations off audio thread [#64](http://github.com/ebu/ear-production-suite/pull/64) [#68](http://github.com/ebu/ear-production-suite/pull/68)
* Fixed bug where changed parameters might not cause DAW to prompt to save changes [#67](http://github.com/ebu/ear-production-suite/pull/67)
* Properly account for tail length when exporting [#71](http://github.com/ebu/ear-production-suite/pull/71)
* Fixed bug where changes were not taking place when switching programmes [#76](http://github.com/ebu/ear-production-suite/pull/76)
* Fixed bug where ‘no name’ items could appear in scene [#78](http://github.com/ebu/ear-production-suite/pull/78)
* Added support for more speaker layouts in DirectSpeaker plugin [#89](http://github.com/ebu/ear-production-suite/pull/89) [#90](http://github.com/ebu/ear-production-suite/pull/90) [#109](http://github.com/ebu/ear-production-suite/pull/109) [#112](http://github.com/ebu/ear-production-suite/pull/112) [#127](http://github.com/ebu/ear-production-suite/pull/127)
* Fixed crash when using ADM extension with a REAPER language pack [#91](http://github.com/ebu/ear-production-suite/pull/91)
* Added 2+7+0 monitoring plugin [#108](http://github.com/ebu/ear-production-suite/pull/108)
* Removed redundant metadata updates [#111](http://github.com/ebu/ear-production-suite/pull/111) [#113](http://github.com/ebu/ear-production-suite/pull/113) [#135](http://github.com/ebu/ear-production-suite/pull/135) [#142](http://github.com/ebu/ear-production-suite/pull/142) [#145](http://github.com/ebu/ear-production-suite/pull/145)
* Fixed memory leak when changing speaker layout [#116](http://github.com/ebu/ear-production-suite/pull/116)
* Fixed combo box initialisation issues [#118](http://github.com/ebu/ear-production-suite/pull/118)
* Version information now show in plugins and extension [#120](http://github.com/ebu/ear-production-suite/pull/120)
* Added support for ADM with two character language codes [#121](http://github.com/ebu/ear-production-suite/pull/121)
* Fixed crash due to recursive mutex locking [#133](http://github.com/ebu/ear-production-suite/pull/133)
* Fixed issue where scene would remove incorrect items [#87](http://github.com/ebu/ear-production-suite/pull/87)
* Preliminary arm64 support for Apple Silicon [#137](http://github.com/ebu/ear-production-suite/pull/137) [#148](http://github.com/ebu/ear-production-suite/pull/148)
* Use patched BW64 library [#149](http://github.com/ebu/ear-production-suite/pull/149)
* Support ITU-R BS.2076-2 structures and time formats [#151](http://github.com/ebu/ear-production-suite/pull/151)
* Fix automation points ordering issue on import [#153](http://github.com/ebu/ear-production-suite/pull/153)
* Various CI improvements
