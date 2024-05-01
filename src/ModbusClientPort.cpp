#include "ModbusClientPort.h"

#include "ModbusPort.h"

namespace Modbus {

struct ClientPort::RequestParams
{
    void *object;
};

ClientPort::ClientPort(Port *port)
{
    m_state = STATE_UNKNOWN;
    m_currentClient = nullptr;
    port->setServerMode(false);
    m_port = port;
    m_repeats = 0;
    m_settings.repeatCount = 1;
}

ClientPort::~ClientPort()
{
    delete m_port;
}

Type ClientPort::type() const
{
    return m_port->type();
}

StatusCode ClientPort::close()
{
    StatusCode s = Status_Good;
    if (m_port->isOpen())
    {
        s = m_port->close();
        m_currentClient = nullptr;
    }
    return s;
}

bool ClientPort::isOpen() const
{
    return m_port->isOpen();
}

StatusCode ClientPort::request(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff, uint16_t maxSzBuff, uint16_t *szOutBuff)
{
    m_port->writeBuffer(unit, func, buff, szInBuff);
    StatusCode r = process();
    if (r == Status_Processing)
        return r;
    if (StatusIsBad(r))
    {
        setStatus(r);
        m_repeats++;
        if (m_repeats < m_settings.repeatCount)
        {
            m_port->setNextRequestRepeated(true);
            return Status_Processing;
        }
    }
    m_repeats = 0;
    m_currentClient = nullptr;
    if (StatusIsBad(r))
    {
        setStatus(r);
        return r;
    }
    r = m_port->readBuffer(unit, func, buff, maxSzBuff, szOutBuff);
    setStatus(r);
    return r;
}

StatusCode ClientPort::process()
{
    StatusCode r;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_BEGIN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
            r = m_port->open();
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
            {
                if (r != Status_Processing) // an error occured
                    m_port->freeWriteBuffer(); // mark the buffer is free to store new data
                return r;
            }
            m_state = STATE_OPENED;
            fRepeatAgain = true;
            break;
        case STATE_WAIT_FOR_CLOSE:
            r = m_port->close();
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
                return r;
            m_state = STATE_CLOSED;
            //fRepeatAgain = true;
            break;
        case STATE_OPENED:
        case STATE_BEGIN_WRITE:
            // send data to server
            if (m_port->isChanged())
            {
                m_state = STATE_WAIT_FOR_CLOSE;
                fRepeatAgain = true;
                break;
            }
            if (!m_port->isOpen())
            {
                m_state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            m_state = STATE_WRITE;
            // no need break
        case STATE_WRITE:
            r = m_port->write();
            if (r != Status_Good) // if not OK it's mean that an error occured or in process
            {
                if (r != Status_Processing) // an error occured
                {
                    m_state = STATE_BEGIN_WRITE;
                    m_port->freeWriteBuffer(); // mark the buffer is free to store new data
                }
                return r;
            }
            m_state = STATE_BEGIN_READ;
            // no need break
        case STATE_BEGIN_READ:
        case STATE_READ:
            r = m_port->read();
            if (r != Status_Processing) // if process finished (Good or Bad)
            {
                m_port->freeWriteBuffer(); // mark the buffer is free to store new data
                m_state = STATE_BEGIN_WRITE;
                return r;
            }
            break;
        default:
            if (m_port->isOpen())
                m_state = STATE_OPENED;
            else
                m_state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    } 
    while (fRepeatAgain);
    return Status_Processing;
}

ClientPort::RequestParams *ClientPort::createRequestParams(void *obj)
{
    RequestParams *rp = new RequestParams;
    rp->object = obj;
    return rp;
}

void ClientPort::deleteRequestParams(RequestParams *rp)
{
    delete rp;
}

ClientPort::RequestStatus ClientPort::getRequestStatus(RequestParams *rp)
{
    if (m_currentClient)
    {
        if (m_currentClient == rp->object)
            return Process;
        return Disable;
    }
    else
    {
        m_currentClient = rp->object;
        return Enable;
    }
}

void ClientPort::cancelRequest(RequestParams* rp)
{
    if (m_currentClient == rp->object)
        m_currentClient = nullptr;
}

} // namespace Chain 
