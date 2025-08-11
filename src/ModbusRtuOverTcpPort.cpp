#include "ModbusRtuOverTcpPort.h"
#include "ModbusTcpPort_p.h"

ModbusRtuOverTcpPort::ModbusRtuOverTcpPort(ModbusTcpSocket *socket, bool blocking) :
    ModbusRtuPort(ModbusTcpPortPrivate::create(socket, blocking))
{
}

ModbusRtuOverTcpPort::ModbusRtuOverTcpPort(bool blocking) :
    ModbusRtuPort(ModbusTcpPortPrivate::create(nullptr, blocking))
{
}
