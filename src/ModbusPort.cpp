#include "ModbusPort.h"
#include "ModbusPort_p.h"

ModbusPort::ModbusPort(ModbusPortPrivate *d) :
    d_ptr(d)
{
}

ModbusPort::~ModbusPort()
{
    delete d_ptr;
}

void ModbusPort::setNextRequestRepeated(bool /*v*/)
{
}

bool ModbusPort::isChanged() const
{
    return d_ptr->changed;
}

bool ModbusPort::isServerMode() const
{
    return d_ptr->isServerMode();
}

void ModbusPort::setServerMode(bool mode)
{
    d_ptr->setServerMode(mode);
}

bool ModbusPort::isBlocking() const
{
    return d_ptr->isBlocking();
}

bool ModbusPort::isNonBlocking() const
{
    return d_ptr->isNonBlocking();
}

uint32_t ModbusPort::timeout() const
{
    return d_ptr->timeout();
}

void ModbusPort::setTimeout(uint32_t timeout)
{
    if (d_ptr->timeout() != timeout)
    {
        d_ptr->setTimeout(timeout);
        d_ptr->setChanged(true);
    }
}

uint32_t ModbusPort::timeoutInterByte() const
{    
    return d_ptr->timeoutInterByte();
}

void ModbusPort::setTimeoutInterByte(uint32_t timeout)
{
    if (d_ptr->timeoutInterByte() != timeout)
    {
        d_ptr->setTimeoutInterByte(timeout);
        d_ptr->setChanged(true);
    }
}

StatusCode ModbusPort::lastErrorStatus() const
{
    return d_ptr->lastErrorStatus();
}

const Modbus::Char *ModbusPort::lastErrorText() const
{
    return d_ptr->lastErrorText();
}

StatusCode ModbusPort::setError(StatusCode status, const Char *text)
{
    return d_ptr->setError(status, String(text));
}
