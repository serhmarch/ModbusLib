#include "ModbusClientPort.h"
#include "ModbusClientPort_p.h"

#include "ModbusPort.h"

struct ModbusClientPort::RequestParams
{
    void *object;
};

ModbusClientPort::ModbusClientPort(ModbusPort *port) :
    ModbusObject(new ModbusClientPortPrivate)
{
    port->setServerMode(false);
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    d->state = STATE_UNKNOWN;
    d->currentClient = nullptr;
    d->port = port;
    d->repeats = 0;
    d->settings.repeatCount = 1;
}

ModbusClientPort::~ModbusClientPort()
{
    delete d_ModbusClientPort(d_ptr)->port;
}

Modbus::Type ModbusClientPort::type() const
{
    return d_ModbusClientPort(d_ptr)->port->type();
}

Modbus::StatusCode ModbusClientPort::close()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    Modbus::StatusCode s = Status_Good;
    if (d->port->isOpen())
    {
        s = d->port->close();
        d->currentClient = nullptr;
    }
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

Modbus::StatusCode ModbusClientPort::lastStatus() const
{
    return d_ModbusClientPort(d_ptr)->lastStatus;
}

const Char *ModbusClientPort::lastErrorText() const
{
    return d_ModbusClientPort(d_ptr)->port->lastErrorText();
}

Modbus::StatusCode ModbusClientPort::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    d->port->writeBuffer(unit, func, buff, szInBuff);
    Modbus::StatusCode r = process();
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

Modbus::StatusCode ModbusClientPort::process()
{
    ModbusClientPortPrivate *d = d_ModbusClientPort(d_ptr);
    Modbus::StatusCode r;
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
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
            {
                if (r != Status_Processing) // an error occured
                    d->port->freeWriteBuffer(); // mark the buffer is free to store new data
                return r;
            }
            d->state = STATE_OPENED;
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = d->port->close();
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
                return r;
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
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
            {
                if (r != Status_Processing) // an error occured
                {
                    d->state = STATE_BEGIN_WRITE;
                    d->port->freeWriteBuffer(); // mark the buffer is free to store new data
                }
                return r;
            }
            d->state = STATE_BEGIN_READ;
            // no need break
        case STATE_BEGIN_READ:
        case STATE_READ:
            r = d->port->read();
            if (r != Status_Processing) // if process finished (Good or Bad)
            {
                d->port->freeWriteBuffer(); // mark the buffer is free to store new data
                d->state = STATE_BEGIN_WRITE;
                return r;
            }
            break;
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

const void *ModbusClientPort::currentClient() const
{
    return d_ModbusClientPort(d_ptr)->currentClient;
}

ModbusClientPort::RequestParams *ModbusClientPort::createRequestParams(void *obj)
{
    RequestParams *rp = new RequestParams;
    rp->object = obj;
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
