#-------------------------------------------------
#
# Project created by QtCreator 2012-07-17T17:21:10
#
#-------------------------------------------------

QT  += core gui
QT  += network
QT  += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = updateDVB
TEMPLATE = app

SOURCES += dvb_settings.cpp
SOURCES += dvr_thread.cpp
SOURCES += demux_file.cpp
SOURCES += tuning.cpp
SOURCES += blindscan.cpp
SOURCES += iqplot.cpp
SOURCES += dvbstream_thread.cpp
SOURCES += blindscan_thread.cpp
SOURCES += blindscan_save.cpp
SOURCES += tuning_thread.cpp
SOURCES += dvbtune.cpp
SOURCES += settings.cpp
SOURCES += demux_dvr.cpp
SOURCES += scan.cpp
SOURCES += mainwindow.cpp
SOURCES += main.cpp

HEADERS  += dvb_settings.h
HEADERS  += dvr_thread.h
HEADERS  += demux_file.h
HEADERS  += tuning.h
HEADERS  += blindscan.h
HEADERS  += iqplot.h
HEADERS  += dvbstream_thread.h
HEADERS  += tuning_thread.h
HEADERS  += blindscan_thread.h
HEADERS  += blindscan_save.h
HEADERS  += dvbtune.h
HEADERS  += settings.h
HEADERS  += demux_dvr.h
HEADERS  += scan.h
HEADERS  += mainwindow.h

FORMS    += mainwindow.ui \
    settings.ui \
    demux_dvr.ui \
    blindscan_save.ui \
    tuning.ui \
    blindscan.ui \
    iqplot.ui \
    demux_file.ui

INCLUDEPATH += /usr/local/qwt-6.1.3-svn/include
INCLUDEPATH += /usr/include/qwt
LIBS += -Wl,-rpath,/usr/local/qwt-6.1.3-svn/lib -L /usr/local/qwt-6.1.3-svn/lib -l:libqwt.so

OTHER_FILES +=

TARGET.path = /usr/local/bin
TARGET.files = updateDVB
TARGET.extra = ln -sf /usr/local/bin/updateDVB ~/Desktop/updateDVB
INSTALLS += TARGET
