#include "ModbusAscOverTcpPort.h"
#include "ModbusTcpPortBase_p.h"
#include "ModbusAscFrame_p.h"

ModbusAscOverTcpPort::ModbusAscOverTcpPort(ModbusSocket *socket, bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusAscFramePrivate(), socket, blocking))
{
}

ModbusAscOverTcpPort::ModbusAscOverTcpPort(bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusAscFramePrivate(), nullptr, blocking))
{
}
