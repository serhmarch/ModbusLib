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
    Modbus_test.cpp \
    ModbusClientPort_test.cpp \
    ModbusServerPort_test.cpp \
    ModbusTcpPort_test.cpp \
    ModbusRtuPort_test.cpp \
    ModbusAscPort_test.cpp \
    main.cpp
