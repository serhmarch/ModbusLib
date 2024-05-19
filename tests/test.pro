include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

unix:QMAKE_RPATHDIR += .

DESTDIR = ../bin

INCLUDEPATH += . ..   \
    $$PWD/../src

LIBS += -L../bin -lmodbus

SOURCES += \
        TestModbus.cpp \
        main.cpp
