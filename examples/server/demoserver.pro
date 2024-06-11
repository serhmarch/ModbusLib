TEMPLATE = app

CONFIG += c++17
CONFIG -= qt
CONFIG += skip_target_version_ext
CONFIG += console
#CONFIG -= app_bundle

unix:QMAKE_RPATHDIR += .

DESTDIR = ../../bin

INCLUDEPATH += . ..   \
    $$PWD/../../src
    
SOURCES += \
    $$PWD/demoserver.cpp

LIBS += -L../../bin -lmodbus

