BmsONE

1. What is this

BmsONE is an editor for bmson files.
Bmson is a new music game format derived from BMS. See [6. Links] for more information about bmson.

The latest version of this document can be found at following URL:
http://sky.geocities.jp/exclusion_bms/bmsone.html


2. How to Use

2.1 Caution

The current version lacks a lot of necessary features. Furthermore, it often crashes or behaves wrong.
Be sure to make backups and save files frequently.


2.2 Adding Sound Channels

[Channel] -> [Add] and then select sound files. Wav and Ogg Vorbis are supported.
Dragging and dropping sound files also works.


2.3 Locating Sound Notes

First select a sound channel in which notes are located by clicking on the corresponding BGM lane.
Then Shift+Click on a (playing or BGM) lane to locate a new note which plays sound from beginning.
You can locate a note which slices sound by just clicking on the lane.
Right-clicking on an existing note will delete it.

Waveforms drawn in a BGM lane stand for the sound played at the time.
Colors of waveforms tell you the sound is played by key notes or BGM notes.


2.4 Other Features

In [Info] window, you can input basic information of the song.
Initial BPM should be set before notes are located because it affects waveforms and preview sounds.

To edit bar lines, Ctrl+Click on measures bar, the leftmost lane of the sequence view,
where measure numbers are displayed. Bar lines with gray numbers are temporary, that is,
they are added automatically every four beats and can move or become permanent
when another bar line is inserted or removed manually.

To edit BPM events, click on the lane next to measures bar.
After placing or selecting a BPM event, a small tool bar to input BPM appears.

BGA data and most extra data cannot be edited, but they will not be lost by this application.

A configuration file is stored in a directory such as:
  [Windows] C:\Users\<USER>\AppData\Local\BmsONE
  [Mac OS X] ~/Library/Preferences/BmsONE

This application has two locales: English and Japanese.
The language is determined by system settings.


3. License

The author of this software is not liable for any damages caused by the software.
Do not use this software for illegal purposes; for example, you must not create a bmson data
which includes audio or graphical data illegally downloaded or extracted from music CDs or games.
Redistribution of this software is permitted only if the whole archive of the software is
provided for free and kept unchanged.

This software uses Qt under LGPL license. Dlls (for Windows), Frameworks and PlugIns (for Mac)
come from Qt, so you can use the software while replacing them with those of other versions.
See the next section for build information.

Contact me for questions. I will consider supplying source codes if you want.


4. Development Environment

[Windows]
Windows 10 Pro
Desktop Qt 5.5.0 MSVC2012 OpenGL 32bit

[Mac OS X]
Mac OS X 10.10.5
Desktop Qt 5.5.0 clang 64bit

[Common]
Using Xiph.org's libogg 1.3.2 and libvorbis 1.3.5.


5. Contact

exclusion
Twitter: @excln
E-mail:  exclusion_bms@yahoo.co.jp
Web:     http://sky.geocities.jp/exclusion_bms/

Bug reports and proposals are also welcome on Twitter hash tag #bmsone.
Please tweet with #bmson tag for general topics on bmson format.


6. Links

Bmson Project by wosderge http://cerebralmuddystream.nekokan.dyndns.info/bmson/
BMSまとめ @wiki - bmson開発部 http://www40.atwiki.jp/laser_bm/pages/104.html


7. History

Sep 27 2015 alpha 0.0.4
  - Fixed a bug that sequence view goes totally wrong when a sound note in BGM lane is previewed.
  - Unicode characters are now supported also in extra info fields and sound file names.

Sep 26 2015 alpha 0.0.3
  - Upgraded Qt from 5.4.1 to 5.5.0.
  - Implemented some additional UI features.
  - Fixed major known factors of crashes on Mac.
  - Bar lines and BPM events can now be edited.
  - Modified the way to store data so that unsupported data will not be lost.
  - Fixed most wrong behaviors in drawing waveforms.
  - BmsONE now uses a configuration file.

Sep 17 2015 alpha 0.0.2

Sep 12 2015 alpha 0.0.1
