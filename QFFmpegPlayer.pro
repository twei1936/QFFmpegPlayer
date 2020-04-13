#-------------------------------------------------
#
# Project created by QtCreator 2020-04-05T20:47:19
#
#-------------------------------------------------

QT     += core gui opengl openglextensions widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QFFmpegPlayer
TEMPLATE = app

LIBS += -L/usr/local/lib/ffmpeg/ -lavcodec -lavdevice -lavformat -lswscale -lavutil -lswresample

DEPENDPATH += $/usr/local/lib/ffmpeg

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 console

SOURCES += \
        main.cpp \
        QFFmpegPlayer.cpp \
    QFDemux.cpp \
    QFDecode.cpp \
    QFVideoWidget.cpp \
    QFResample.cpp \
    QFAudioPlay.cpp \
    QFAudioThread.cpp \
    QFVideoThread.cpp \
    QFDemuxThread.cpp \
    QFDecodeThread.cpp \
    QFSlider.cpp

HEADERS += \
        QFFmpegPlayer.h \
    QFDemux.h \
    QFDecode.h \
    QFVideoWidget.h \
    QFResample.h \
    QFAudioPlay.h \
    QFAudioThread.h \
    QFVideoThread.h \
    IVideoCall.h \
    QFDemuxThread.h \
    QFDecodeThread.h \
    QFSlider.h

FORMS += \
        QFFmpegPlayer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
