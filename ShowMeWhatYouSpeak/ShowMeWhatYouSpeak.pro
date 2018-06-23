#-------------------------------------------------
#
# Project created by QtCreator 2018-06-20T21:59:33
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShowMeWhatYouSpeak
TEMPLATE = app

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    audioinput.cpp \
    fft.cpp \
    fft4g.c

HEADERS += \
        mainwindow.h \
    audioinput.h \
    fft.h

FORMS += \
        mainwindow.ui

CONFIG += c++11
