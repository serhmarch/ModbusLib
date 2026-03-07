TEMPLATE = lib

include(version.pri)

#CONFIG += c++17
CONFIG += qt

DEFINES += MODBUS_EXPORTS

DESTDIR = ../../bin

unix:QMAKE_RPATHDIR += .

HEADERS +=                          \
    $$PWD/Modbus_config.h           \
    $$PWD/ModbusPlatform.h          \
    $$PWD/ModbusGlobal.h            \
    $$PWD/Modbus.h                  \
    $$PWD/ModbusObject.h            \
    $$PWD/ModbusObject_p.h          \
    $$PWD/ModbusPort.h              \
    $$PWD/ModbusPort_p.h            \
    $$PWD/ModbusFrame_p.h           \
    $$PWD/ModbusRtuFrame_p.h        \
    $$PWD/ModbusAscFrame_p.h        \
    $$PWD/ModbusNetFrame_p.h        \
    $$PWD/ModbusSerialPort.h        \
    $$PWD/ModbusSerialPort_p.h      \
    $$PWD/ModbusRtuPort.h           \
    $$PWD/ModbusAscPort.h           \
    $$PWD/ModbusNetPort.h           \
    $$PWD/ModbusNetPort_p.h         \
    $$PWD/ModbusTcpPortBase.h       \
    $$PWD/ModbusTcpPortBase_p.h     \
    $$PWD/ModbusTcpPort.h           \
    $$PWD/ModbusUdpPortBase.h       \
    $$PWD/ModbusUdpPortBase_p.h     \
    $$PWD/ModbusUdpPort.h           \
    $$PWD/ModbusRtuOverTcpPort.h    \
    $$PWD/ModbusAscOverTcpPort.h    \
    $$PWD/ModbusRtuOverUdpPort.h    \
    $$PWD/ModbusAscOverUdpPort.h    \
    $$PWD/ModbusClientPort.h        \
    $$PWD/ModbusClientPort_p.h      \
    $$PWD/ModbusClient.h            \
    $$PWD/ModbusClient_p.h          \
    $$PWD/ModbusServerPort.h        \
    $$PWD/ModbusServerPort_p.h      \
    $$PWD/ModbusServerResource.h    \
    $$PWD/ModbusServerResource_p.h  \
    $$PWD/ModbusTcpServer.h         \
    $$PWD/ModbusTcpServer_p.h       \

SOURCES +=                          \
    $$PWD/Modbus.cpp                \
    $$PWD/ModbusObject.cpp          \
    $$PWD/ModbusPort.cpp            \
    $$PWD/ModbusSerialPort.cpp      \
    $$PWD/ModbusRtuPort.cpp         \
    $$PWD/ModbusAscPort.cpp         \
    $$PWD/ModbusNetPort.cpp         \
    $$PWD/ModbusTcpPort.cpp         \
    $$PWD/ModbusUdpPort.cpp         \
    $$PWD/ModbusRtuOverTcpPort.cpp  \
    $$PWD/ModbusAscOverTcpPort.cpp  \
    $$PWD/ModbusRtuOverUdpPort.cpp  \
    $$PWD/ModbusAscOverUdpPort.cpp  \
    $$PWD/ModbusClientPort.cpp      \
    $$PWD/ModbusClient.cpp          \
    $$PWD/ModbusServerPort.cpp      \
    $$PWD/ModbusServerResource.cpp  \
    $$PWD/ModbusTcpServer.cpp


contains(CONFIG, qt) {
#    message("Qt support is enabled.")
QT = core
HEADERS +=                          \
    $$PWD/ModbusQt.h                \

SOURCES +=                          \
    $$PWD/ModbusQt.cpp              \

} else {
#    message("Qt support is NOT enabled.")
}

win32 {

HEADERS +=                                  \
    $$PWD/win/Modbus_win.h                  \
    $$PWD/win/ModbusSerialPort_p_win.h      \
    $$PWD/win/ModbusTcpPortBase_p_win.h     \
    $$PWD/win/ModbusUdpPortBase_p_win.h     \
    $$PWD/win/ModbusTcpServer_p_win.h       \

SOURCES +=                                  \
    $$PWD/win/Modbus_win.cpp                \
    $$PWD/win/ModbusSerialPort_win.cpp      \
    $$PWD/win/ModbusTcpPortBase_win.cpp     \
    $$PWD/win/ModbusUdpPortBase_win.cpp     \
    $$PWD/win/ModbusTcpServer_win.cpp       \

LIBS += -lWs2_32
LIBS += -lWinmm
LIBS += -lsetupapi
LIBS += -lAdvapi32

}

unix {

HEADERS +=                                    \
    $$PWD/unix/Modbus_unix.h                  \
    $$PWD/unix/ModbusSerialPort_p_unix.h      \
    $$PWD/unix/ModbusTcpPortBase_p_unix.h     \
    $$PWD/unix/ModbusUdpPortBase_p_unix.h     \
    $$PWD/unix/ModbusTcpServer_p_unix.h       \

SOURCES +=                                    \
    $$PWD/unix/Modbus_unix.cpp                \
    $$PWD/unix/ModbusSerialPort_unix.cpp      \
    $$PWD/unix/ModbusTcpPortBase_unix.cpp     \
    $$PWD/unix/ModbusUdpPortBase_unix.cpp     \
    $$PWD/unix/ModbusTcpServer_unix.cpp       \

}

