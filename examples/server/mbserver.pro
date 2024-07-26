TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

unix:QMAKE_RPATHDIR += .

DESTDIR = ../../bin

INCLUDEPATH += . ..   \
    $$PWD/../../src
    
SOURCES += \
    $$PWD/mbserver.c

LIBS += -L../../bin -lmodbus

