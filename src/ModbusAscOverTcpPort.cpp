#include "ModbusAscOverTcpPort.h"
#include "ModbusTcpPort_p.h"

ModbusAscOverTcpPort::ModbusAscOverTcpPort(ModbusTcpSocket *socket, bool blocking) :
    ModbusAscPort(ModbusTcpPortPrivate::create(socket, blocking))
{
}

ModbusAscOverTcpPort::ModbusAscOverTcpPort(bool blocking) :
    ModbusAscPort(ModbusTcpPortPrivate::create(nullptr, blocking))
{
}
