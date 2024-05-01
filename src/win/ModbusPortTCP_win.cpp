#include "../ModbusPortTCP.h"

#include "ModbusTCP_win.h"

namespace Modbus {

struct PortTCP::PlatformData
{
    TCPSocket *socket;
    DWORD timestamp;
    void *addr;

    inline void freeAddr()
    {
        if (this->addr)
        {
            freeaddrinfo(reinterpret_cast<ADDRINFO*>(this->addr));
            this->addr = nullptr;
        }
    }

};

void PortTCP::constructorPrivate(TCPSocket *socket)
{
    WSADATA data;
    WSAStartup(0x202, &data);

    m_platformData = new PlatformData;
    m_platformData->timestamp = 0;
    m_platformData->addr = nullptr;

    if (socket)
    {
        socket->setBlocking(isBlocking());
        m_platformData->socket = socket;
    }
    else
    {
        m_platformData->socket = new TCPSocket();
    }

}

void PortTCP::destructorPrivate()
{
    m_platformData->freeAddr();
    delete m_platformData;
    WSACleanup();
}

Handle PortTCP::handle() const
{
    return reinterpret_cast<Handle>(m_platformData->socket->socket());
}

Modbus::StatusCode PortTCP::open()
{
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (m_state)
        {
        case STATE_BEGIN:
        case STATE_CLOSED:
        {
            clearChanged();
            if (isOpen())
            {
                m_state = STATE_OPENED;
                return Status_Good;
            }
            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            m_platformData->freeAddr();

            ADDRINFO* addr = nullptr;
            DWORD dwRetval = getaddrinfo(m_host.data(), NULL, &hints, &addr);
            if (dwRetval != 0)
                return setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info"));
            m_platformData->addr = addr;
            m_platformData->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_platformData->socket->isInvalid())
            {
                m_platformData->freeAddr();
                return setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket"));
            }
            m_platformData->socket->setBlocking(isBlocking());
            if (isBlocking())
                m_platformData->socket->setTimeout(m_timeout);
            reinterpret_cast<sockaddr_in*>(reinterpret_cast<ADDRINFO*>(m_platformData->addr)->ai_addr)->sin_port = htons(m_port);
            m_platformData->timestamp = GetTickCount();
            m_state = STATE_WAIT_FOR_OPEN;
        }
        // no need break
        case STATE_WAIT_FOR_OPEN:
        {
            int r = m_platformData->socket->connect(reinterpret_cast<ADDRINFO*>(m_platformData->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFOW*>(m_platformData->addr)->ai_addrlen));
            if ((r == 0) || (WSAGetLastError() == WSAEISCONN))
            {
                m_state = STATE_BEGIN;
                return Status_Good;
            }
            else if (isNonBlocking() && (GetTickCount() - m_platformData->timestamp >= m_timeout))
            {
                m_platformData->socket->close();
                m_state = STATE_CLOSED;
                return setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting - timeout"));
            }
            else if (isBlocking())
            {
                m_platformData->socket->close();
                m_state = STATE_CLOSED;
                return setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting. Error code: ") + toString(WSAGetLastError()));
            }
        }
            break;
        default:
            if (!isOpen())
            {
                m_state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode PortTCP::close()
{
    if (!m_platformData->socket->isInvalid())
    {
        m_platformData->socket->shutdown();
        m_platformData->socket->close();
    }
    m_state = STATE_CLOSED;
    return Status_Good;
}

bool PortTCP::isOpen() const
{
    if (m_platformData->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = m_platformData->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode PortTCP::write()
{
    switch (m_state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        int c = m_platformData->socket->send(reinterpret_cast<char*>(m_buff), m_sz, 0);
        if (c > 0)
        {
            emitTx(m_buff, m_sz);
            m_state = STATE_BEGIN;
            return Status_Good;
        }
        else
        {
            close();
            return setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing"));
        }
    }
        break;
    default:
        break;
    }
    return Status_Processing;
}

StatusCode PortTCP::read()
{
    const uint16_t size = MBCLIENTTCP_BUFF_SZ;
    switch (m_state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_READ:
        m_platformData->timestamp = GetTickCount();
        m_state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        int c = m_platformData->socket->recv(reinterpret_cast<char*>(m_buff), size, 0);
        if (c > 0)
        {
            m_sz = static_cast<uint16_t>(c);
            emitRx(m_buff, m_sz);
            m_state = STATE_BEGIN;
            return Status_Good;
        }
        else if (c == 0)
        {
            close();
            return setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading - remote connection closed"));
        }
        else if (isNonBlocking() && (GetTickCount() - m_platformData->timestamp >= m_timeout)) // waiting timeout read first byte elapsed
        {
            close();
            return setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading - timeout"));
        }
        else
        {
            int e = WSAGetLastError();
            if (e != WSAEWOULDBLOCK)
            {
                close();
                return setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading. Error code: ") + toString(e));
            }
        }
    }
        break;
    default:
        break;
    }
    return Status_Processing;
}


} // namespace Modbus
