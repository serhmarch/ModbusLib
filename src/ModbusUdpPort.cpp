#include "ModbusUdpPort.h"
#include "ModbusUdpPortBase_p.h"
#include "ModbusNetFrame_p.h"

ModbusUdpPort::ModbusUdpPort(bool blocking) :
    ModbusUdpPortBase(ModbusUdpPortBasePrivate::create(new ModbusNetFramePrivate(), blocking))
{
}

void ModbusUdpPort::setNextRequestRepeated(bool v)
{
    d_net(d_ptr->frame)->autoIncrement = !v;
}

bool ModbusUdpPort::autoIncrement() const
{
    return d_net(d_ptr->frame)->autoIncrement;
}

uint16_t ModbusUdpPort::transactionId() const
{
    return d_net(d_ptr->frame)->transaction;
}

void ModbusUdpPort::setTransactionId(uint16_t id)
{
    d_net(d_ptr->frame)->transaction = id;
}
