TEMPLATE = app

TARGET = showmewhatyouspeak

QT       += multimedia widgets

SOURCES  += main.cpp \
            engine.cpp \
            frequencyspectrum.cpp \
            mainwidget.cpp \
            settingsdialog.cpp \
            spectrograph.cpp \
            spectrumanalyser.cpp \
            utils.cpp

HEADERS  += engine.h \
            frequencyspectrum.h \
            mainwidget.h \
            settingsdialog.h \
            spectrograph.h \
            spectrumanalyser.h \
            utils.h

INCLUDEPATH += ../fftreal
DEPENDPATH += $${INCLUDEPATH}

macx {
    LIBS += -F../fftreal
    LIBS += -framework fftreal
} else {
    LIBS += -L../build
    LIBS += -lfftreal
}
DESTDIR = ../build

RESOURCES += \
    icons.qrc

