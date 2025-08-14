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
    return d_ptr->changed;
}

bool ModbusPort::isServerMode() const
{
    return d_ptr->modeServer;
}

void ModbusPort::setServerMode(bool mode)
{
    d_ptr->modeServer = mode;
}

bool ModbusPort::isBlocking() const
{
    return d_ptr->modeBlocking;
}

bool ModbusPort::isNonBlocking() const
{
    return !d_ptr->modeBlocking;
}

Handle ModbusPort::handle() const
{
    return d_ptr->handle();
}

StatusCode ModbusPort::open()
{
    return d_ptr->open();
}

StatusCode ModbusPort::close()
{
    return d_ptr->close();
}

bool ModbusPort::isOpen() const
{
    return d_ptr->isOpen();
}

StatusCode ModbusPort::write()
{
    return d_ptr->write();
}

StatusCode ModbusPort::read()
{
    return d_ptr->read();
}

void ModbusPort::setNextRequestRepeated(bool v)
{
    d_ptr->setNextRequestRepeated(v);
}

const Char *ModbusPort::host() const
{
    return d_ptr->host().data();
}

void ModbusPort::setHost(const Char *host)
{
    if (d_ptr->host() != host)
    {
        d_ptr->setHost(host);
        d_ptr->setChanged(true);
    }
}

uint16_t ModbusPort::port() const
{
    return d_ptr->port();
}

void ModbusPort::setPort(uint16_t port)
{
    if (d_ptr->port() != port)
    {
        d_ptr->setPort(port);
        d_ptr->setChanged(true);
    }
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

const Char *ModbusPort::portName() const
{
    return d_ptr->portName().data();
}

void ModbusPort::setPortName(const Char *portName)
{
    if (d_ptr->portName() != portName)
    {
        d_ptr->setPortName(portName);
        d_ptr->setChanged(true);
    }
}

int32_t ModbusPort::baudRate() const
{
    return d_ptr->baudRate();
}

void ModbusPort::setBaudRate(int32_t baudRate)
{
    if (d_ptr->baudRate() != baudRate)
    {
        d_ptr->setBaudRate(baudRate);
        d_ptr->setChanged(true);
    }
}

int8_t ModbusPort::dataBits() const
{
    return d_ptr->dataBits();
}

void ModbusPort::setDataBits(int8_t dataBits)
{
    if (d_ptr->dataBits() != dataBits)
    {
        d_ptr->setDataBits(dataBits);
        d_ptr->setChanged(true);
    }
}

Parity ModbusPort::parity() const
{
    return d_ptr->parity();
}

void ModbusPort::setStopBits(StopBits stopBits)
{
    if (d_ptr->stopBits() != stopBits)
    {
        d_ptr->setStopBits(stopBits);
        d_ptr->setChanged(true);
    }
}

FlowControl ModbusPort::flowControl() const
{
    return d_ptr->flowControl();
}

void ModbusPort::setParity(Parity parity)
{
    if (d_ptr->parity() != parity)
    {
        d_ptr->setParity(parity);
        d_ptr->setChanged(true);
    }
}

StopBits ModbusPort::stopBits() const
{
    return d_ptr->stopBits();
}

void ModbusPort::setFlowControl(FlowControl flowControl)
{
    if (d_ptr->flowControl() != flowControl)
    {
        d_ptr->setFlowControl(flowControl);
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

const uint8_t *ModbusPort::readBufferData() const
{
    return d_ptr->buff;
}

uint16_t ModbusPort::readBufferMaxSize() const
{
    return d_ptr->c_buffSz;
}

uint16_t ModbusPort::readBufferSize() const
{
    return d_ptr->sz;
}

const uint8_t *ModbusPort::writeBufferData() const
{
    return d_ptr->buff;
}

uint16_t ModbusPort::writeBufferMaxSize() const
{
    return d_ptr->c_buffSz;
}

uint16_t ModbusPort::writeBufferSize() const
{
    return d_ptr->sz;
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
    if (szInBuff > d_ptr->c_buffSz)
        return d_ptr->setError(Status_BadWriteBufferOverflow, StringLiteral("Write-buffer overflow"));
    memcpy(d_ptr->buff, buff, szInBuff);
    d_ptr->sz = szInBuff;
    return Status_Good;
}

StatusCode ModbusPort::readRawBuffer(void *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    uint16_t sz = d_ptr->sz;
    if (sz > maxSzBuff)
        return d_ptr->setError(Status_BadReadBufferOverflow, StringLiteral("Read-buffer overflow"));
    memcpy(buff, d_ptr->buff, sz);
    *szOutBuff = sz;
    return Status_Good;
}

StatusCode ModbusPort::setError(StatusCode status, const Char *text)
{
    return d_ptr->setError(status, String(text));
}
