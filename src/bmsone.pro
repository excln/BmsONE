#-------------------------------------------------
#
# Project created by QtCreator 2015-08-14T09:15:37
#
#-------------------------------------------------

QT       += core concurrent gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BmsONE
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++

win32: QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:libcmt
win32: QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:libcmt

SOURCES += main.cpp\
        MainWindow.cpp \
    History.cpp \
    SequenceView.cpp \
    Document.cpp \
    InfoView.cpp \
    ChannelInfoView.cpp \
    QuasiModalEdit.cpp \
    AudioPlayer.cpp \
    SequenceTools.cpp \
    Stabilizer.cpp \
    SoundChannel.cpp \
    WaveData.cpp \
    WaveStream.cpp \
    ScrollableForm.cpp \
    Util.cpp \
    BpmEditTool.cpp \
    SelectedObjectView.cpp \
    JsonExtension.cpp \
    SoundChannelInternal.cpp \
	SequenceViewInternal.cpp \
    HistoryUtil.cpp \
	DocumentInfo.cpp \
	QOggVorbisAdapter.cpp \
    SymbolIconManager.cpp \
    SequenceViewContents.cpp \
    StatusBar.cpp \
    SequenceViewCursor.cpp \
    CollapseButton.cpp \
    SequenceViewEditMode.cpp \
    SequenceViewWriteMode.cpp \
    libogg/bitwise.c \
    libogg/framing.c \
    libvorbis/analysis.c \
    libvorbis/bitrate.c \
    libvorbis/block.c \
    libvorbis/codebook.c \
    libvorbis/envelope.c \
    libvorbis/floor0.c \
    libvorbis/floor1.c \
    libvorbis/info.c \
    libvorbis/lookup.c \
    libvorbis/lpc.c \
    libvorbis/lsp.c \
    libvorbis/mapping0.c \
    libvorbis/mdct.c \
    libvorbis/psy.c \
    libvorbis/registry.c \
    libvorbis/res0.c \
    libvorbis/sharedbook.c \
    libvorbis/smallft.c \
    libvorbis/synthesis.c \
    libvorbis/vorbisenc.c \
    libvorbis/vorbisfile.c \
    libvorbis/window.c \
    SequenceViewPreview.cpp \
    SoundChannelPreview.cpp \
    SequenceViewChannelInternal.cpp \
    Versioning.cpp \
	bmson/Bmson021.cpp \
	bmson/Bmson100.cpp \
	bmson/Bmson100Convert.cpp \
    Bmson.cpp \
    bmson/BmsonConvertDef.cpp \
    Preferences.cpp \
    Skin.cpp \
    ViewMode.cpp \
    MasterCache.cpp \
    MasterView.cpp \
    EditConfig.cpp \
    PrefEdit.cpp \
    ScalarRegion.cpp

HEADERS  += MainWindow.h \
    History.h \
    SequenceView.h \
    Document.h \
    InfoView.h \
    ChannelInfoView.h \
    Wave.h \
    QuasiModalEdit.h \
    AudioPlayer.h \
    SequenceTools.h \
    Stabilizer.h \
    SequenceDef.h \
    ScrollableForm.h \
    UIDef.h \
    BpmEditTool.h \
    SelectedObjectView.h \
    JsonExtension.h \
    SoundChannel.h \
    SoundChannelInternal.h \
    DocumentDef.h \
    SoundChannelDef.h \
	SequenceViewInternal.h \
	HistoryUtil.h \
	QOggVorbisAdapter.h \
    DocumentAux.h \
    SymbolIconManager.h \
    SequenceViewDef.h \
    CollapseButton.h \
    SequenceViewContexts.h \
    libvorbis/books/coupled/res_books_51.h \
    libvorbis/books/coupled/res_books_stereo.h \
    libvorbis/books/floor/floor_books.h \
    libvorbis/books/uncoupled/res_books_uncoupled.h \
    libvorbis/modes/floor_all.h \
    libvorbis/modes/psych_8.h \
    libvorbis/modes/psych_11.h \
    libvorbis/modes/psych_16.h \
    libvorbis/modes/psych_44.h \
    libvorbis/modes/residue_8.h \
    libvorbis/modes/residue_16.h \
    libvorbis/modes/residue_44.h \
    libvorbis/modes/residue_44p51.h \
    libvorbis/modes/residue_44u.h \
    libvorbis/modes/setup_8.h \
    libvorbis/modes/setup_11.h \
    libvorbis/modes/setup_16.h \
    libvorbis/modes/setup_22.h \
    libvorbis/modes/setup_32.h \
    libvorbis/modes/setup_44.h \
    libvorbis/modes/setup_44p51.h \
    libvorbis/modes/setup_44u.h \
    libvorbis/modes/setup_X.h \
    SequenceViewChannelInternal.h \
    Versioning.h \
	bmson/Bmson100.h \
	bmson/Bmson021.h \
	bmson/Bmson100Convert.h \
    Bmson.h \
    bmson/BmsonConvertDef.h \
    Preferences.h \
    Skin.h \
    ViewMode.h \
    MasterCache.h \
    MasterView.h \
    EditConfig.h \
    PrefEdit.h \
    ScalarRegion.h

FORMS    +=

TRANSLATIONS = i18n/ja.ts

RESOURCES += \
    bmsone.qrc

win32: RC_ICONS = bmsone.ico

macx: ICON = bmsone.icns



INCLUDEPATH += $$PWD/ $$PWD/libvorbis
DEPENDPATH += $$PWD/

