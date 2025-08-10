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
    return d_ptr->settings.hostOrPortName.data();
}

void ModbusPort::setHost(const Char *host)
{
    if (d_ptr->settings.hostOrPortName != host)
    {
        d_ptr->settings.hostOrPortName = host;
        d_ptr->setChanged(true);
    }
}

uint16_t ModbusPort::port() const
{
    return d_ptr->settings.port;
}

void ModbusPort::setPort(uint16_t port)
{
    if (d_ptr->settings.port != port)
    {
        d_ptr->settings.port = port;
        d_ptr->setChanged(true);
    }
}

uint32_t ModbusPort::timeout() const
{
    return d_ptr->settings.timeout;
}

void ModbusPort::setTimeout(uint32_t timeout)
{
    if (d_ptr->settings.timeout != timeout)
    {
        d_ptr->settings.timeout = timeout;
        d_ptr->setChanged(true);
    }
}

const Char *ModbusPort::portName() const
{
    return d_ptr->settings.hostOrPortName.data();
}

void ModbusPort::setPortName(const Char *portName)
{
    ModbusPortPrivate *d = d_ptr;
    if (d->settings.hostOrPortName != portName)
    {
        d->settings.hostOrPortName = portName;
        d->setChanged(true);
    }
}

int32_t ModbusPort::baudRate() const
{
    return d_ptr->settings.baudRate;
}

void ModbusPort::setBaudRate(int32_t baudRate)
{
    ModbusPortPrivate *d = d_ptr;
    if (d->settings.baudRate != baudRate)
    {
        d->settings.baudRate = baudRate;
        d->setChanged(true);
    }
}

int8_t ModbusPort::dataBits() const
{
    return d_ptr->settings.dataBits;
}

void ModbusPort::setDataBits(int8_t dataBits)
{
    ModbusPortPrivate *d = d_ptr;
    if (d->settings.dataBits != dataBits)
    {
        d->settings.dataBits = dataBits;
        d->setChanged(true);
    }
}

Parity ModbusPort::parity() const
{
    return d_ptr->settings.parity;
}

void ModbusPort::setStopBits(StopBits stopBits)
{
    ModbusPortPrivate *d = d_ptr;
    if (d->settings.stopBits != stopBits)
    {
        d->settings.stopBits = stopBits;
        d->setChanged(true);
    }
}

FlowControl ModbusPort::flowControl() const
{
    return d_ptr->settings.flowControl;
}

void ModbusPort::setParity(Parity parity)
{
    ModbusPortPrivate *d = d_ptr;
    if (d->settings.parity != parity)
    {
        d->settings.parity = parity;
        d->setChanged(true);
    }
}

StopBits ModbusPort::stopBits() const
{
    return d_ptr->settings.stopBits;
}

void ModbusPort::setFlowControl(FlowControl flowControl)
{
    if (d_ptr->settings.flowControl != flowControl)
    {
        d_ptr->settings.flowControl = flowControl;
        d_ptr->setChanged(true);
    }
}

uint32_t ModbusPort::timeoutInterByte() const
{    return d_ptr->settings.timeoutInterByte;
}

void ModbusPort::setTimeoutInterByte(uint32_t timeout)
{
    if (d_ptr->settings.timeoutInterByte != timeout)
    {
        d_ptr->settings.timeoutInterByte = timeout;
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

StatusCode ModbusPort::writeRawBuffer(const uint8_t *buff, uint16_t szInBuff)
{
    if (szInBuff > d_ptr->c_buffSz)
        return d_ptr->setError(Status_BadWriteBufferOverflow, StringLiteral("Write-buffer overflow"));
    memcpy(d_ptr->buff, buff, szInBuff);
    d_ptr->sz = szInBuff;
    return Status_Good;
}

StatusCode ModbusPort::readRawBuffer(uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    uint16_t sz = d_ptr->sz;
    if (sz > maxSzBuff)
        return d_ptr->setError(Status_BadReadBufferOverflow, StringLiteral("Read-buffer overflow"));
    memcpy(buff, d_ptr->buff, sz);
    return Status_Good;
}

StatusCode ModbusPort::setError(StatusCode status, const Char *text)
{
    return d_ptr->setError(status, String(text));
}
