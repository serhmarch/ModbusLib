#include "../ModbusTcpPort.h"

#include "ModbusTcpPort_p_win.h"

ModbusTcpPort::ModbusTcpPort(ModbusTcpSocket *socket, bool blocking) :
    ModbusPort(new ModbusTcpPortPrivateWin(socket, blocking))
{
}

ModbusTcpPort::ModbusTcpPort(bool blocking) :
    ModbusTcpPort(nullptr, blocking)
{
}

ModbusTcpPort::~ModbusTcpPort()
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
}

Handle ModbusTcpPort::handle() const
{
    return reinterpret_cast<Handle>(d_win(d_ptr)->socket->socket());
}

Modbus::StatusCode ModbusTcpPort::open()
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        {
            d->clearChanged();
            if (isOpen())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            d->freeAddr();

            ADDRINFO* addr = nullptr;
            DWORD status = getaddrinfo(d->settings.host.data(), NULL, &hints, &addr);
            if (status != 0)
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info for '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(status) +
                                                        StringLiteral(". ") + getLastErrorText());
            d->addr = addr;
            d->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (d->socket->isInvalid())
            {
                d->freeAddr();
                int err = WSAGetLastError();
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket for '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(err) +
                                                        StringLiteral(". ") + getLastErrorText());
            }
            d->socket->setBlocking(false); // Note: in case of block-socket it will be set after connect
            if (isBlocking())
                d->socket->setTimeout(this->timeout());
            reinterpret_cast<sockaddr_in*>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr)->sin_port = htons(d->settings.port);
            d->timestamp = GetTickCount();
            d->state = STATE_WAIT_FOR_OPEN;
        }
        // no need break
        case STATE_WAIT_FOR_OPEN:
        {
            if (isNonBlocking())
            {
                int r = d->socket->connect(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addrlen));
                DWORD err = WSAGetLastError();
                if ((r == 0) || (err == WSAEISCONN))
                {
                    d->state = STATE_OPENED;
                    return Status_Good;
                }
                else if (GetTickCount() - d->timestamp >= this->timeout())
                {
                    d->socket->close();
                    d->state = STATE_CLOSED;
                    return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                            StringLiteral("'. Timeout") );
                }
            }
            else if (isBlocking())
            {
                int r = d->socket->connect(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addrlen));
                if (r != 0)
                {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                    // Use select() to wait for writability (i.e., successful connect)
                    fd_set writefds;
                    FD_ZERO(&writefds);
                    FD_SET(d->socket->socket(), &writefds);

                    TIMEVAL tv;
                    tv.tv_sec = this->timeout() / 1000;
                    tv.tv_usec = (this->timeout() % 1000) * 1000;

                    r = select(0, NULL, &writefds, NULL, &tv);
                    if (r <= 0)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        if (r == 0)
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                    StringLiteral("'. Timeout") );
                        else
                        {
                            int err = WSAGetLastError();
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                    StringLiteral("'. Error code: ") + toModbusString(err) +
                                                                    StringLiteral(". ") + getLastErrorText());
                        }
                    }

                    // Check for errors after select
                    int sockErr = 0;
                    socklen_t len = sizeof(sockErr);
                    getsockopt(d->socket->socket(), SOL_SOCKET, SO_ERROR, (char*)&sockErr, &len);
                    if (sockErr != 0)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        return d->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(sockErr) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                }
                // Success - set blocking mode
                d->socket->setBlocking(true);
                d->state = STATE_OPENED;
                return Status_Good;
            }
        }
            break;
        default:
            if (!isOpen())
            {
                d->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            d->state = STATE_OPENED;
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusTcpPort::close()
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusTcpPort::isOpen() const
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    if (d->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = d->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode ModbusTcpPort::write()
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    switch (d->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        int c = d->socket->send(reinterpret_cast<char*>(d->buff), d->sz, 0);
        if (c > 0)
        {
            d->state = STATE_OPENED;
            return Status_Good;
        }
        else
        {
            close();
            DWORD err = WSAGetLastError();
            return d->setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing to '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                   StringLiteral("'. Error code: ") + toModbusString(err) +
                                                   StringLiteral(". ") + getLastErrorText());
        }
    }
        break;
    default:
        break;
    }
    return Status_Processing;
}

StatusCode ModbusTcpPort::read()
{
    ModbusTcpPortPrivateWin *d = d_win(d_ptr);
    const uint16_t size = MBCLIENTTCP_BUFF_SZ;
    switch (d->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_READ:
        d->timestamp = GetTickCount();
        d->state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        int c = d->socket->recv(reinterpret_cast<char*>(d->buff), size, 0);
        if (c > 0)
        {
            d->sz = static_cast<uint16_t>(c);
            d->state = STATE_OPENED;
            return Status_Good;
        }
        else if (c == 0)
        {
            close();
            // Note: When connection is remotely closed is not error for server side
            if (d->modeServer)
                return Status_Uncertain;
            else
                return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                      StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (GetTickCount() - d->timestamp >= this->timeout())) // waiting timeout read first byte elapsed
        {
            close();
            return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                  StringLiteral("'. Timeout") );
        }
        else
        {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                close();
                return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->settings.host + StringLiteral(":") + toModbusString(d->settings.port) +
                                                      StringLiteral("'. Error code: ") + toModbusString(err) +
                                                      StringLiteral(". ") + getLastErrorText());
            }
        }
    }
        break;
    default:
        break;
    }
    return Status_Processing;
}
