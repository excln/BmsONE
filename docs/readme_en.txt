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
Right-clicking on an existing note will delete the note.

Waveforms drawn in a BGM lane stand for the sound played at the time.
Colors of waveforms tell you the sound is played by key notes or BGM notes.
Note that waveform drawing functions have a lot of bugs.


2.4 Other Features

In [Info] window, you can input basic information of the song.
Initial BPM should be set before notes are located because it affects waveforms and preview sounds.

Currently, the functionalities on BPM events, long notes and bar lines are passive, that is,
these objects cannot be edited but can be shown if the file already contains them.
Bar lines are automatically added every four beats.

BGA data cannot even be loaded. Thus existing BGA data will be lost if edited by this application.

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
Desktop Qt 5.4.1 MSVC2012 OpenGL 32bit

[Mac]
Mac OS X 10.9.5
Desktop Qt 5.4.1 clang 64bit

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

Sep 17 2015 alpha 0.0.2
Sep 12 2015 alpha 0.0.1
