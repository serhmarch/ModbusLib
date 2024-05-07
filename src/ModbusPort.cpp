#include "ModbusPort.h"
#include "ModbusPort_p.h"

void ModbusPort::setNextRequestRepeated(bool /*v*/)
{
}

bool ModbusPort::isChanged() const
{
    return d_ModbusPort(d_ptr)->changed;
}

bool ModbusPort::isServerMode() const
{
    return d_ModbusPort(d_ptr)->modeServer;
}

void ModbusPort::setServerMode(bool mode)
{
    d_ModbusPort(d_ptr)->modeServer = mode;
}

bool ModbusPort::isBlocking() const
{
    return d_ModbusPort(d_ptr)->modeSynch;
}

bool ModbusPort::isNonBlocking() const
{
    return !d_ModbusPort(d_ptr)->modeSynch;
}

const Modbus::Char *ModbusPort::lastErrorText() const
{
    return d_ModbusPort(d_ptr)->lastErrorText.data();
}

bool ModbusPort::isWriteBufferBlocked() const
{
    return d_ModbusPort(d_ptr)->block;
}

void ModbusPort::freeWriteBuffer()
{
    d_ModbusPort(d_ptr)->block = false;
}

void ModbusPort::emitTx(const uint8_t *buff, uint16_t size)
{
    this->emitSignal(&ModbusPort::emitTx, buff, size);
}

void ModbusPort::emitRx(const uint8_t *buff, uint16_t size)
{
    this->emitSignal(&ModbusPort::emitRx, buff, size);
}

StatusCode ModbusPort::setError(StatusCode status, const Char *text)
{
    return d_ModbusPort(d_ptr)->setError(status, String(text));
}

void ModbusPort::setChanged(bool changed)
{
    d_ModbusPort(d_ptr)->setChanged(changed);
}
