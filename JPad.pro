QT      += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
HEADERS += dialog.h

QMAKE_CXXFLAGS = -std=c++11
SOURCES += dialog.cpp \
           main.cpp

RESOURCES += \
    resources.qrc
TARGET = JPad
