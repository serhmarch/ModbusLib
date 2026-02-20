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

bool ModbusPort::isChanged() const
{
    return d_ptr->isChanged();
}

bool ModbusPort::isServerMode() const
{
    return d_ptr->modeServer();
}

void ModbusPort::setServerMode(bool server)
{
    d_ptr->setModeServer(server);
}

bool ModbusPort::isBlocking() const
{
    return d_ptr->isBlocking();
}

bool ModbusPort::isNonBlocking() const
{
    return !d_ptr->isBlocking();
}

uint32_t ModbusPort::timeout() const
{
    return d_ptr->settingsBase.timeout;
}

void ModbusPort::setTimeout(uint32_t timeout)
{
    d_ptr->settingsBase.timeout = timeout;
}

void ModbusPort::setNextRequestRepeated(bool)
{
    // Note: Base implementation does nothing.
    // This function is used only for TCP/UDP version of the Modbus protocol.
}

const uint8_t *ModbusPort::readBufferData() const
{
    return d_ptr->buff();
}

uint16_t ModbusPort::readBufferMaxSize() const
{
    return d_ptr->buffMaxSize();
}

uint16_t ModbusPort::readBufferSize() const
{
    return d_ptr->buffSize();
}

const uint8_t *ModbusPort::writeBufferData() const
{
    return d_ptr->buff();
}

uint16_t ModbusPort::writeBufferMaxSize() const
{
    return d_ptr->buffMaxSize();
}

uint16_t ModbusPort::writeBufferSize() const
{
    return d_ptr->buffSize();
}

StatusCode ModbusPort::lastErrorStatus() const
{
    return d_ptr->lastErrorStatus();
}

const Modbus::Char *ModbusPort::lastErrorText() const
{
    return d_ptr->lastErrorText();
}

StatusCode ModbusPort::writeRawBuffer(const void *buff, uint16_t szInBuff)
{
    if (szInBuff > d_ptr->buffMaxSize())
        return d_ptr->setError(Status_BadWriteBufferOverflow, StringLiteral("Write-buffer overflow"));
    memcpy(d_ptr->buff(), buff, szInBuff);
    d_ptr->frame->sz = szInBuff;
    return Status_Good;
}

StatusCode ModbusPort::readRawBuffer(void *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    uint16_t sz = d_ptr->buffSize();
    if (sz > maxSzBuff)
        return d_ptr->setError(Status_BadReadBufferOverflow, StringLiteral("Read-buffer overflow"));
    memcpy(buff, d_ptr->buff(), sz);
    *szOutBuff = sz;
    return Status_Good;
}

StatusCode ModbusPort::writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff)
{
    return d_ptr->writeBuffer(unit, func, buff, szInBuff);
}

StatusCode ModbusPort::readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    return d_ptr->readBuffer(unit, func, buff, maxSzBuff, szOutBuff);
}

StatusCode ModbusPort::setError(StatusCode status, const Char *text)
{
    return d_ptr->setError(status, String(text));
}
