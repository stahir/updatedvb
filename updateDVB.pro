#-------------------------------------------------
#
# Project created by QtCreator 2012-07-17T17:21:10
#
#-------------------------------------------------

QT  += core gui
QT  += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = updateDVB
TEMPLATE = app

SOURCES += dvb_settings.cpp \
    tuning.cpp \
    blindscan.cpp \
    iqplot.cpp
SOURCES += dvbstream_thread.cpp
SOURCES +=
SOURCES += blindscan_thread.cpp
SOURCES += blindscan_save.cpp
SOURCES += tuning_thread.cpp
SOURCES +=
SOURCES += dvbtune.cpp
SOURCES += settings.cpp
SOURCES += demux_file.cpp
SOURCES += demux_dvr.cpp
SOURCES += scan.cpp
SOURCES += mainwindow.cpp
SOURCES += main.cpp

HEADERS  += dvb_settings.h \
    tuning.h \
    blindscan.h \
    iqplot.h
HEADERS  += dvbstream_thread.h
HEADERS  += tuning_thread.h
HEADERS  +=
HEADERS  += blindscan_thread.h
HEADERS  += blindscan_save.h
HEADERS  +=
HEADERS  += dvbtune.h
HEADERS  += settings.h
HEADERS  += demux_file.h
HEADERS  += demux_dvr.h
HEADERS  += scan.h
HEADERS  += mainwindow.h

FORMS    += mainwindow.ui \
    settings.ui \
    demux_file.ui \
    demux_dvr.ui \
    blindscan_save.ui \
    tuning.ui \
    blindscan.ui \
    iqplot.ui

INCLUDEPATH += /usr/include/qwt
INCLUDEPATH += /opt/local/include/qwt
INCLUDEPATH += /usr/local/qwt-6.1.0/include
LIBS += -L /usr/local/qwt-6.1.0/lib -l qwt

OTHER_FILES +=
