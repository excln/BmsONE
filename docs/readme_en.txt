BmsONE

1. What is this

BmsONE is an editor for bmson files.
Bmson is a new music game format derived from BMS. See [6. Links] for more information about bmson.

This application currently supports v1.0 and v0.21 bmson, but some features cannot be edited yet.

The latest version of this document can be found at following URL:
http://sky.geocities.jp/exclusion_bms/bmsone.html


2. How to Use

2.1 Caution

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

Bmson Project by wosderge
  http://cerebralmuddystream.nekokan.dyndns.info/bmson/
  The official web site of bmson format.

BMSまとめ @wiki - how to bmson
  http://www40.atwiki.jp/laser_bm/pages/110.html
  How to create and play bmson. (Japanese)

bmson specs by flicknote
  https://docs.google.com/document/d/1ZDjfjWud8UG3RPjyhN-dd1rVjPaactcMT3PIODTap9s/edit
  A draft and detailed explanation of bmson format specification. (English)
  Note that this includes many features not supported by bmson applications yet.

#bmson Creation Notes by ドルフィン
  https://docs.google.com/document/d/1gQKPWApeL03aO09-II7slxTeuvm3HO_FmY1D4chRvOQ/edit
  How to create bmson, including key sounds creation with DAWs and editing with bmson editors. (English)


7. History

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
