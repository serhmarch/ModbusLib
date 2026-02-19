#include "ModbusRtuOverTcpPort.h"
#include "ModbusTcpPortBase_p.h"
#include "ModbusRtuFrame_p.h"

ModbusRtuOverTcpPort::ModbusRtuOverTcpPort(ModbusSocket *socket, bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusRtuFramePrivate(), socket, blocking))
{
}

ModbusRtuOverTcpPort::ModbusRtuOverTcpPort(bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusRtuFramePrivate(), nullptr, blocking))
{
}
