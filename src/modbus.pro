TEMPLATE = lib

#CONFIG += c++17
CONFIG -= qt

DEFINES += MODBUS_EXPORTS

DESTDIR = ../bin

unix:QMAKE_RPATHDIR += .

HEADERS +=                         \
    $$PWD/Modbus_platform.h        \
    $$PWD/Modbus.h                 \
    $$PWD/ModbusCallback.h         \
    $$PWD/ModbusPort.h             \
    $$PWD/ModbusPortTCP.h          \
    $$PWD/ModbusPortSerial.h       \
    $$PWD/ModbusPortRTU.h          \
    $$PWD/ModbusPortASC.h          \
    $$PWD/ModbusClientPort.h       \
    $$PWD/ModbusClient.h           \
    $$PWD/ModbusServerPort.h       \
    $$PWD/ModbusServerResource.h   \
    $$PWD/ModbusServerTCP.h        \

SOURCES +=                         \
    $$PWD/Modbus.cpp               \
    $$PWD/ModbusPort.cpp           \
    $$PWD/ModbusPortTCP.cpp        \
    $$PWD/ModbusPortSerial.cpp     \
    $$PWD/ModbusPortRTU.cpp        \
    $$PWD/ModbusPortASC.cpp        \
    $$PWD/ModbusClientPort.cpp     \
    $$PWD/ModbusClient.cpp         \
    $$PWD/ModbusServerPort.cpp     \
    $$PWD/ModbusServerResource.cpp \
    $$PWD/ModbusServerTCP.cpp      \

win32 {

HEADERS += \
    $$PWD/win/ModbusTCP_win.h

SOURCES += \
    $$PWD/win/ModbusPortTCP_win.cpp    \
    $$PWD/win/ModbusServerTCP_win.cpp  \
    $$PWD/win/ModbusPortSerial_win.cpp \
    $$PWD/win/Modbus_win.cpp           \

LIBS += -lWs2_32
LIBS += -lWinmm

}

