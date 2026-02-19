#include "ModbusTcpPort.h"
#include "ModbusTcpPortBase_p.h"
#include "ModbusNetFrame_p.h"

ModbusTcpPort::ModbusTcpPort(ModbusSocket *socket, bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusNetFramePrivate(), socket, blocking))
{
}

ModbusTcpPort::ModbusTcpPort(bool blocking) :
    ModbusTcpPortBase(ModbusTcpPortBasePrivate::create(new ModbusNetFramePrivate(), nullptr, blocking))
{
}

void ModbusTcpPort::setNextRequestRepeated(bool v)
{
    d_net(d_ptr->frame)->autoIncrement = !v;
}

bool ModbusTcpPort::autoIncrement() const
{
    return d_net(d_ptr->frame)->autoIncrement;
}

uint16_t ModbusTcpPort::transactionId() const
{
    return d_net(d_ptr->frame)->transaction;
}

void ModbusTcpPort::setTransactionId(uint16_t id)
{
    d_net(d_ptr->frame)->transaction = id;
}
