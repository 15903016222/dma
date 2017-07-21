QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AscanDemo
TEMPLATE = app


HEADERS += \
    a_scan_hwidget.h \
    a_scan_widget.h \
    mythread.h \
    dma.h

SOURCES += \
    a_scan_hwidget.cpp \
    a_scan_widget.cpp \
    main.cpp \
    mythread.cpp \
    dma_doppler-II.cpp
