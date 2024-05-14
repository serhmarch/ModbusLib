#include "ModbusClientPort.h"
#include "ModbusClientPort_p.h"

#include "ModbusPort.h"


struct ModbusClientPort::RequestParams
{
    ModbusObject *object;
};

ModbusClientPort::ModbusClientPort(ModbusPort *port) :
    ModbusObject(new ModbusClientPortPrivate(port))
{
}

ModbusClientPort::~ModbusClientPort()
{
    delete d_ModbusClientPort(d_ptr)->port;
}

Type ModbusClientPort::type() const
{
    return d_ModbusClientPort(d_ptr)->port->type();
}

StatusCode ModbusClientPort::close()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    StatusCode s = Status_Good;
    s = d->port->close();
    if (StatusIsGood(s))
        signalClosed(d->getName());
    d->currentClient = nullptr;
    return s;
}

bool ModbusClientPort::isOpen() const
{
    return d_ModbusClientPort(d_ptr)->port->isOpen();
}

uint32_t ModbusClientPort::repeatCount() const
{
    return d_ModbusClientPort(d_ptr)->settings.repeatCount;
}

void ModbusClientPort::setRepeatCount(uint32_t v)
{
    if (v > 0)
        d_ModbusClientPort(d_ptr)->settings.repeatCount = v;
}

ModbusPort *ModbusClientPort::port() const
{
    return d_ModbusClientPort(d_ptr)->port;
}

StatusCode ModbusClientPort::lastStatus() const
{
    return d_ModbusClientPort(d_ptr)->lastStatus;
}

const Char *ModbusClientPort::lastErrorText() const
{
    return d_ModbusClientPort(d_ptr)->port->lastErrorText();
}

StatusCode ModbusClientPort::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    d->port->writeBuffer(unit, func, buff, szInBuff);
    StatusCode r = process();
    if (r == Status_Processing)
        return r;
    if (StatusIsBad(r))
    {
        d->setStatus(r);
        d->repeats++;
        if (d->repeats < d->settings.repeatCount)
        {
            d->port->setNextRequestRepeated(true);
            return Status_Processing;
        }
    }
    d->repeats = 0;
    d->currentClient = nullptr;
    if (StatusIsBad(r))
    {
        d->setStatus(r);
        return r;
    }
    r = d->port->readBuffer(unit, func, buff, maxSzBuff, szOutBuff);
    d->setStatus(r);
    return r;
}

StatusCode ModbusClientPort::process()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_BEGIN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
            r = d->port->open();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r)) // an error occured
            {
                signalError(d->getName(), r, d->port->lastErrorText());
                d->port->freeWriteBuffer(); // mark the buffer is free to store new data
                return r;
            }
            d->state = STATE_OPENED;
            signalOpened(d->getName());
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = close();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))
            {
                signalError(d->getName(), r, d->port->lastErrorText());
                return r;
            }
            d->state = STATE_CLOSED;
            //fRepeatAgain = true;
            break;
        case STATE_OPENED:
        case STATE_BEGIN_WRITE:
            // send data to server
            if (d->port->isChanged())
            {
                d->state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            if (!d->port->isOpen())
            {
                d->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            d->state = STATE_WRITE;
            // no need break
        case STATE_WRITE:
            r = d->port->write();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r)) // an error occured
            {
                signalError(d->getName(), r, d->port->lastErrorText());
                d->port->freeWriteBuffer(); // mark the buffer is free to store new data
                d->state = STATE_BEGIN_WRITE;
                return r;
            }
            else
                signalTx(d->getName(), d->port->writeBufferData(), d->port->writeBufferSize());
            d->state = STATE_BEGIN_READ;
            // no need break
        case STATE_BEGIN_READ:
        case STATE_READ:
            r = d->port->read();
            if (StatusIsProcessing(r))
                return r;
            if (StatusIsBad(r))
                signalError(d->getName(), r, d->port->lastErrorText());
            else
                signalRx(d->getName(), d->port->readBufferData(), d->port->readBufferSize());
            d->port->freeWriteBuffer(); // mark the buffer is free to store new data
            d->state = STATE_BEGIN_WRITE;
            return r;
        default:
            if (d->port->isOpen())
                d->state = STATE_OPENED;
            else
                d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

const ModbusObject *ModbusClientPort::currentClient() const
{
    return d_ModbusClientPort(d_ptr)->currentClient;
}

ModbusClientPort::RequestParams *ModbusClientPort::createRequestParams(ModbusObject *object)
{
    RequestParams *rp = new RequestParams;
    rp->object = object;
    return rp;
}

void ModbusClientPort::deleteRequestParams(RequestParams *rp)
{
    delete rp;
}

ModbusClientPort::RequestStatus ModbusClientPort::getRequestStatus(RequestParams *rp)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    if (d->currentClient)
    {
        if (d->currentClient == rp->object)
            return Process;
        return Disable;
    }
    else
    {
        d->currentClient = rp->object;
        return Enable;
    }
}

void ModbusClientPort::cancelRequest(RequestParams* rp)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    if (d->currentClient == rp->object)
        d->currentClient = nullptr;
}

void ModbusClientPort::signalOpened(const Modbus::Char *source)
{
    emitSignal(&ModbusClientPort::signalOpened, source);
}

void ModbusClientPort::signalClosed(const Modbus::Char *source)
{
    emitSignal(&ModbusClientPort::signalClosed, source);
}

void ModbusClientPort::signalTx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(&ModbusClientPort::signalTx, source, buff, size);
}

void ModbusClientPort::signalRx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
{
    emitSignal(&ModbusClientPort::signalRx, source, buff, size);
}

void ModbusClientPort::signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text)
{
    emitSignal(&ModbusClientPort::signalError, source, status, text);
}
