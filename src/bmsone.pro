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
    Bmson.cpp \
    BpmEditTool.cpp \
    SelectedObjectView.cpp

HEADERS  += MainWindow.h \
    History.h \
    Bmson.h \
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
    SelectedObjectView.h

FORMS    +=

TRANSLATIONS = i18n/ja.ts

RESOURCES += \
    bmsone.qrc

win32: RC_ICONS = bmsone.ico

macx: ICON = bmsone.icns



win32: INCLUDEPATH += $$PWD/
win32: DEPENDPATH += $$PWD/

win32: LIBS += -L$$PWD/lib/win32_VS2012/ -llibogg_static

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/libogg_static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/liblibogg_static.a

win32: LIBS += -L$$PWD/lib/win32_VS2012/ -llibvorbis_static

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/libvorbis_static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/liblibvorbis_static.a

win32: LIBS += -L$$PWD/lib/win32_VS2012/ -llibvorbisfile_static


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/libvorbisfile_static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/lib/win32_VS2012/liblibvorbisfile_static.a



macx: INCLUDEPATH += /usr/local/Cellar/libogg/1.3.2/include /usr/local/Cellar/libvorbis/1.3.5/include
macx: LIBS += /usr/local/Cellar/libogg/1.3.2/lib/libogg.a \
     /usr/local/Cellar/libvorbis/1.3.5/lib/libvorbis.a \
     /usr/local/Cellar/libvorbis/1.3.5/lib/libvorbisfile.a
macx: PRE_TARGETDEPS += /usr/local/Cellar/libogg/1.3.2/lib/libogg.a \
     /usr/local/Cellar/libvorbis/1.3.5/lib/libvorbis.a \
     /usr/local/Cellar/libvorbis/1.3.5/lib/libvorbisfile.a

