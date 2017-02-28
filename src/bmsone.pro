#-------------------------------------------------
#
# Project created by QtCreator 2015-08-14T09:15:37
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bmsone
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        MainWindow.cpp \
    History.cpp \
    BmsonIo.cpp \
    SequenceView.cpp \
    Document.cpp \
    Sequence.cpp \
    InfoView.cpp \
    ChannelInfoView.cpp

HEADERS  += MainWindow.h \
    History.h \
    Bmson.h \
    BmsonIo.h \
    SequenceView.h \
    Document.h \
    InfoView.h \
    ChannelInfoView.h

FORMS    +=

TRANSLATIONS = i18n/ja.ts

RESOURCES += \
    bmsone.qrc
