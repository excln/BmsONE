BmsONE

1. What is this

BmsONE is an editor for bmson files.
Bmson is a new music game format derived from BMS. See [6. Links] for more information about bmson.

This application currently supports v1.0 and v0.21 bmson, but some features cannot be edited yet.

The latest version of this document can be found at following URL:
http://sky.geocities.jp/exclusion_bms/bmsone.html


2. How to Use

2.1 Caution

If an error like ”MSVCP140.dll is missing” occurs on Windows, then please install
Visual C++ Redistributable for Visual Studio 2015 at the following URL.
(Please choose "x86" regardless of your machine.)
https://www.microsoft.com/en-us/download/details.aspx?id=48145

This software is under development, so the current version lacks some features
and often goes wrong (particularly on Mac).
Be sure to make backups and save files frequently.

Note that "Ctrl key" below means Command key (⌘) on Mac.


2.2 Adding Sound Channels

[Channel] -> [Add] and then select sound files. Wav and Ogg Vorbis are supported.
Dragging and dropping sound files also works.


2.3 Locating and Editing Sound Notes

There are two modes, "Edit Mode" and "Write Mode".

In order to locate sound notes, you first have to switch to "Write Mode"
by clicking on the pencil icon in the tool bar.
Next click on a BGM lane whose channel you want to edit. The selected channel is shown highlighted.
Then Shift+Click on a (playing or BGM) lane to locate a new note which plays sound from beginning.
You can locate a note which slices sound by just clicking on the lane.
Right-clicking on an existing note will delete it.

Notice that waveforms are drawn in a BGM lane, which stand for the sound played at that time.
Colors of waveforms tell you the sound is played by key notes or BGM notes.

In "Edit Mode", you can select multiple objects by dragging around them.
This behavior can be modified by Ctrl, Alt and Shift keys.
For example, Alt+dragging selects notes only in the current sound channel.
You can also move notes horizontally by dragging on them, or change their length by Shift+dragging.

You can play sounds in the current channel at that time while Alt+Right-clicking or Middle-clicking.
(If you use Mac, Control(not ⌘)+Clicking works as Right-clicking.)
Clicking on Master Lane previews the whole song.
If you press Ctrl during a preview, then the screen will be scrolled following the playing position.


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
Only extra data in BMSInfo can be edited by writing fragments of JSON directly as follows:
  "updated_at": "14:00 2015/11/14",
  "registered_with": "The Forgetalia"

A configuration file is stored in a directory such as:
  [Windows] C:\Users\<USER>\AppData\Local\BmsONE
  [Mac OS X] ~/Library/Preferences/BmsONE

If this application often crashes when scrolling Sequence View, please choose [View] -
[Sound Channel Lane Display] - [Simple] so that drawing process becomes stable though
waveforms are not displayed.

If you feel hard to edit objects in bmson with a number of channels, then try the following:
  * Select [View] - [Sound Channel Lane Display] - [Compact] or [Simple] to make lanes thin.
  * Check [Channel] - [Find] - [Active Channels Only] and [Show Hit Channels Only] to hide
    channels that are not playing near the current position.


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
Desktop Qt 5.7.0 MSVC2015 OpenGL 32bit

[Mac OS X]
Mac OS X 10.10.5
Desktop Qt 5.7.0 clang 64bit

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

bmson / Bemuse Project by wosderge
  https://bmson.nekokan.dyndns.info/
  The official web site of bmson format.

BMSまとめ @wiki - how to bmson
  http://www40.atwiki.jp/laser_bm/pages/110.html
  How to create and play bmson. (Japanese)

bmson specs by flicknote
  http://bmson-spec.readthedocs.io/en/master/
  A draft and detailed explanation of bmson format specification. (English)

#bmson Creation Notes by ドルフィン
  https://docs.google.com/document/d/1gQKPWApeL03aO09-II7slxTeuvm3HO_FmY1D4chRvOQ/edit
  How to create bmson, including key sounds creation with DAWs and editing with bmson editors. (English)


7. History

Feb 12 2017 beta 0.1.5
  - You can edit "title_image" and "preview_music" in the Information window, which are in bmson 1.0.0 spec.
  - You can change UI font. The default font on Windows was changed.
  - Fixed crashes when previewing sound channels across BPM changes.
  - Fixed crashes when many BPM events are edited at once.
  - You can edit extra data of each sound note and BPM event.
  - You can choose styles "Compact" and "Simple" of sound channel lane display in Sequence View.
  - Improved channel search so that inactive channels can be filtered and Sequence View can show hit channels only.
  - Implemented WAV export.

Dec 25 2016 beta 0.1.4
  - Fixed a problem with invoking external viewers that contain white spaces in their paths.
  - Fixed some bugs in commands "Move to BGM Lanes" and "Move to Key Lanes".
  - Modified influence of "Snap to Grid" on hit test in Edit Mode.
  - Modified display in Sequence View.
  - Sequence View does not prohibit notes from overlapping but instead show warnings.
  - Sequence View supports layered notes editing.
  - Press number keys to move selected notes to specific lanes.
  - You can choose "Compact" format so that the BMSON file is as small as possible.
  - Provisional support for HiDPI displays.

Dec 11 2016 beta 0.1.3
  - Implemented preview on Mini Map.
  - Fixed waveform display that had been inverted in Master Lane and Mini Map.
  - Modified waveform display in Channel Info View.
  - Fixed memory leaks that occurred when new documents are opened.
  - Implemented double clicks on notes to switch them between playable and BGM lanes.
  - Drag to input a long note in Write Mode.
  - Added View Modes for Generic 6,7 Keys and the plain View Mode.
  - Implemented functions to invoke external viewers.
  - Fix a bug in updating Master Cache when notes are edited.
  - Some minor bug fixes and improvements on UI.

Sep 6 2016 beta 0.1.2
  - Upgraded Qt to 5.7.0.
  - Added a View Mode for Circular Rhythm.
  - Added a setting whether notes in inactive channels are shown dark.
  - Implemented auto-scrolling in channel preview, and improved display functionality.
  - Added a setting of the max duration of note preview.
  - Implemented channel search.
  - The mode hint can be chosen out of typical values, and it automatically changes View Mode.

Nov 20 2015 beta 0.1.1
  - Fixed a bug that a new document lacks version info.
  - Reduced unnecessary Master Cache updates during editing sound notes.

Nov 14 2015 beta 0.1.0
  - Improved View Mode functionality.
  - Implemented Master Cache, Master Lane and Mini Map.
  - Support of editing bmson v1.0 data. (Now default output format is v1.0.)

Nov 4 2015 alpha 0.0.6
  - Fixed crashes when opening ogg files on Mac.
  - Reduced crashes when scrolling notecharts on Mac.
  - Implemented previews of a sound channel at arbitrary position.
  - Tentative support of bmson v1.0 input/output.
  - Tentative support of view modes besides 7-key.
  - Implemented Preferences window where you can select languages.
  - Disabled shortcuts while dragging notes (which cause problems in the undo buffer and so on).

Oct 17 2015 alpha 0.0.5
  - Implemented Undo/Redo.
  - Implemented "Edit Mode", and then you can move notes and make long notes.
  - Selected notes can now be deleted or switch playable lanes and BGM lanes at a time.
  - Modified the UI of Info View and Channel View.
  - Fixed some slight bugs and improved performance.

Sep 27 2015 alpha 0.0.4
  - Fixed a bug that sequence view goes totally wrong when a sound note in BGM lane is previewed.
  - Unicode characters are now supported also in extra info fields and sound file names.
  - D&D files into the EXE file now works (Windows only).

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
