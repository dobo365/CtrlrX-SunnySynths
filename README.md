CtrlrX-SunnySynths
=====

CtrlrX-SunnySynths is an alternative fork of CtrlrX done by Damien Selleier which is itself an alternative fork of Roman Kubiak's Ctrlr.

It contains some updates and serves as my test bench (Github / Juce / Visual Studio syncs), Wiki draft... for CtrlrX.

The CtrlrX project is ONLY aimed at delivering updates, a wiki, documentation, tutorials or anything that the community cannot share on the original Ctrlr github due to credential restrictions. 
Let's keep the Ctrlr github alive and keep up with what we were all doing there. But for anything that deserves special credentials unavailable there, just let's do it here on CtrlrX.


CtrlrX ToDo
----

* VST Host must get the panel name while exported as VST3 plugin (currently always named as Ctrlr | Instigator)
* APP/Plugin identifiers properly updated from panel infos in exported instances package. (info.plist, .exe, vst3, etc)
* Implementation of the entire JUCE ValueTree Class with LUA
* File path needs to be updated on save while the panel ID is changed (Save vs. Save as...)
* Re-generating UID must update resource path automatically
* Modulators located within a tabs must have their properties updated while the tab properties are changed 
* Allow earlier "Undo" after a modulator has been deleted
* Export instance process must be improved (dialog alerts, overwriting existing file...)
* Upgrade LUABIND to another LUA/C++ library
* Update to the latest version of JUCE 7


CtrlrX Changelog (only the last version)
---------

#### Version 5.6.30.1
* Missing JUCE File Class definitions bound to LUA
* New LookAndFeel_V4 colourScheme added (V4 JetBlack, V4 YamDX, V4 AkAPC, V4 AkMPC, V4 LexiBlue, V4 KurzGreen, V4 KorGrey, V4 KorGold, V4 ArturOrange, V4 AiraGreen).
* Colours fixed in the LUA Method Editor and LUA Console
* File>Save As removes panelDirty asterisk suffix
* uiButton & uiImageButton can show the MIDI Monitor window by selecting it from the componentInternalFunction property
* Legacy mode for older panels protects their background colours
* Close button added to LUA Method Editor Tabs (as in 5.1.198, 5.2 & 5.3 versions)
* LUA Method Editor Tabs won't shrink and will show a + sign if the TabBar exceeds the window W
* Close button added to Panel Editor Tabs (as in 5.1.198, 5.2 & 5.3 versions)
* Panel Editor Tabs won't shrink and will show a + sign if the TabBar exceeds the window W



# About Ctrlr

Control any MIDI enabled hardware: synthesizers, drum machines, samplers, effects. Create custom User Interfaces. Host them as VST or AU plugins in your favorite DAWs.


Cross Platform
--------------
Works on Windows (XP and up, both 64 and 32bit binaries are available), macOS (10.5 and up), Linux (any modern distro should run it).
Host in your DAW

Each platform has a VST build of Ctrlr so you can host your panels as normal VST plugins, for macOS a special AU build is available.

Customize
---------
Each Panel can be customized by the user, the panels are XML files, every panel can be Edited in Ctrlr to suite your specific needs.

Open Source
-----------
Need special functionality or want to propose a patch/feature update, know a bit about C++/JUCE framework etc. You can always download the source code and build Ctrlr by yourself.

Extend
------
With the scripting possibilities inside Ctrlr you can extend you panels in various ways. The LUA scripting language gives you access to all panels elements and hooks to various events.


# How to build Ctrlr


## Windows

Summary will be added here in the future, links to pdf of build guides by @bijlevel can be found here

[Compiling on Windows 10](https://godlike.com.au/fileadmin/godlike/techtools/ctrlr/guides/Compiling_Ctrlr_for_Windows_10_v2.1.pdf)


## macOS

[Compiling on OSX](https://godlike.com.au/fileadmin/godlike/techtools/ctrlr/guides/My_guide_to_compiling_Ctrlr_for_macOS__Mojave__v2.pdf)


## Linux

A build.sh script is provided in Builds/Generated/Linux/Standalone, a symlink of that
script is location in Builds/Generated/Linux/VST and is used to create a precompiled header
and then to trigger the build using make. You can do that manualy if you like just have
a look at the script, it's really simple.

A more complex solution exists in Scripts/post-commit, this script will build all solutions
for the current architecture, it will also prepare the system for the build, unpack boost.zip
check some packages (on Ubuntu only for now) and start the build. At the end it will create
a self extracting Ctrlr.sh file in Packaging/Linux (create using makeself.sh), it will also
try to scp it to ctrlr.org but that will fail without the correct ssh key, you can just comment
out the scp line in post-commit.

The post-commit script takes an argument "clean" if you wish to clean all the intermididate
files before building. If you want to ignore any package errors that it reports (i assume you
know your system better then my script) then just add -f as an option when building.

# How to export plugin instances (AU, AUv3, VST & VST3)

## VST2 Support
Since Steinberg has discontinued the VST2 API we no longer distribute a VST2. If you are a licensee to the VST2SDK, though, you can still build it. 
The first thing is to be sure to check the path to the VST2 sdk (only available from Steinberg's VST3 directory sdk) in Projucer before calling any script builds.

## VST3 Support
Currently VST3 instances of CTRLR panels are not working properly because CTRLR is not able to generate different VST3 compliant plugin identifiers. 
Unfortunately, exported VST3 instances of your panel will always be named after CTRLR | Instigator. 
The only way to get the correct identifiers for a panel project is to force them at the core during the building step of the VST3 in Xcode/VS/IDE.
To export properly identified VST3 plugins it is then required to build a different stock CTRLR VST3 plugin with JUCE Projucer and Xcode/VS/IDE. 
However, this alternative version of CTRLR VST3 will have the desired panel/plugin identifiers predefined in the Projucer settings. (Plugin Name, Manufacturer Name, Plugin ID, Manufacturer ID etc). 
This intermediate VST3 plugin will then be able to export a final VST3 version of the panel with the proper identifiers. 

## AU Support
Tutorial coming soon

## AUv3 Support
Tutorial coming soon

# Credits
* Thanks to @romankubiak for developing Ctrlr
* Links to contributors coming soon

# Notes
* VST is a registered trademark of Steinberg Media Technologies GmbH
