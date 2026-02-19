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

HEADERS += \
    TestModbus.h \
    MockModbusPort.h \
    MockModbusDevice.h

SOURCES += \
    Modbus_test.cpp \
    ModbusAddress_test.cpp \
    ModbusClientPort_test.cpp \
    ModbusServerPort_test.cpp \
    ModbusServerResource_test.cpp \
    ModbusTcpPort_test.cpp \
    ModbusUdpPort_test.cpp \
    ModbusTcpServer_test.cpp \
    ModbusRtuPort_test.cpp \
    ModbusAscPort_test.cpp \
    ModbusRtuOverTcpPort_test.cpp \
    ModbusAscOverTcpPort_test.cpp \
    ModbusRtuOverUdpPort_test.cpp \
    ModbusAscOverUdpPort_test.cpp \
    main.cpp

win32 {

SOURCES += \
    $$PWD/unix/TestModbus_win.cpp

}

unix {

SOURCES += \
    $$PWD/unix/TestModbus_unix.cpp
    
}

LIBS += -L../bin -lmodbus
