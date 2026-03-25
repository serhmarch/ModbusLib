#include "../ModbusTcpPort.h"

#include "ModbusTcpPortBase_p_win.h"

ModbusTcpPortBasePrivate *ModbusTcpPortBasePrivate::create(ModbusFramePrivate *f, ModbusSocket *socket, bool blocking)
{
    return new ModbusTcpPortBasePrivateWin(f, socket, blocking);
}

Modbus::Handle ModbusTcpPortBase::handle() const
{
    return reinterpret_cast<Handle>(d_win(d_ptr)->socket->socket());
}

Modbus::StatusCode ModbusTcpPortBase::open()
{
    ModbusTcpPortBasePrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        {
            if (this->isOpen())
            {
                if (d->isChanged())
                    this->close();
                else
                {
                    d->state = STATE_OPENED;
                    return Status_Good;
                }
            }
            d->clearChanged();
            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            d->freeAddr();

            ADDRINFO* addr = nullptr;
            DWORD status = getaddrinfo(d->host().data(), NULL, &hints, &addr);
            if (status != 0)
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info for '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                        StringLiteral("'. Error code: ") + toModbusString(status) +
                                                        StringLiteral(". ") + getLastErrorText());
            d->addr = addr;
            d->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (d->socket->isInvalid())
            {
                d->freeAddr();
                int err = WSAGetLastError();
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket for '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                        StringLiteral("'. Error code: ") + toModbusString(err) +
                                                        StringLiteral(". ") + getLastErrorText());
            }
            d->socket->setBlocking(false); // Note: in case of block-socket it will be set after connect
            if (d->isBlocking())
                d->socket->setTimeout(d->timeout());
            reinterpret_cast<sockaddr_in*>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr)->sin_port = htons(d->port());
            d->timestamp = GetTickCount();
            d->state = STATE_WAIT_FOR_OPEN;
        }
            MB_FALLTHROUGH
        case STATE_WAIT_FOR_OPEN:
        {
            if (d->isNonBlocking())
            {
                int r = d->socket->connect(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addrlen));
                DWORD err = WSAGetLastError();
                if ((r == 0) || (err == WSAEISCONN))
                {
                    d->state = STATE_OPENED;
                    return Status_Good;
                }
                else if (GetTickCount() - d->timestamp >= d->timeout())
                {
                    d->socket->close();
                    d->state = STATE_CLOSED;
                    return d->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                             StringLiteral("'. Timeout") );
                }
            }
            else if (d->isBlocking())
            {
                int r = d->socket->connect(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(d->addr)->ai_addrlen));
                if (r != 0)
                {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                                StringLiteral("'. Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                    // Use select() to wait for writability (i.e., successful connect)
                    fd_set writefds;
                    FD_ZERO(&writefds);
                    FD_SET(d->socket->socket(), &writefds);

                    TIMEVAL tv;
                    tv.tv_sec = d->timeout() / 1000;
                    tv.tv_usec = (d->timeout() % 1000) * 1000;

                    r = select(0, NULL, &writefds, NULL, &tv);
                    if (r <= 0)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        if (r == 0)
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                                    StringLiteral("'. Timeout") );
                        else
                        {
                            int err = WSAGetLastError();
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
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
                        return d->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
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
            if (isOpen() && !d->isChanged())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            d->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

Modbus::StatusCode ModbusTcpPortBase::close()
{
    ModbusTcpPortBasePrivateWin *d = d_win(d_ptr);
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusTcpPortBase::isOpen() const
{
    ModbusTcpPortBasePrivateWin *d = d_win(d_ptr);
    if (d->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = d->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

Modbus::StatusCode ModbusTcpPortBase::write()
{
    ModbusTcpPortBasePrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_OPENED:
        case STATE_PREPARE_TO_WRITE:
        case STATE_WAIT_FOR_WRITE:
        case STATE_WAIT_FOR_WRITE_ALL:
        {
            int c = d->socket->send(reinterpret_cast<char*>(d->buff()), d->buffSize(), 0);
            if (c > 0)
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            else
            {
                close();
                DWORD err = WSAGetLastError();
                return d->setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                       StringLiteral("'. Error code: ") + toModbusString(err) +
                                                       StringLiteral(". ") + getLastErrorText());
            }
        }
            break;
        default:
            if (this->isOpen())
            {
                d->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                return d->setError(Status_BadTcpWrite, StringLiteral("Internal state error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

Modbus::StatusCode ModbusTcpPortBase::read()
{
    ModbusTcpPortBasePrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_OPENED:
        case STATE_PREPARE_TO_READ:
            d->timestamp = GetTickCount();
            d->state = STATE_WAIT_FOR_READ;
            MB_FALLTHROUGH
        case STATE_WAIT_FOR_READ:
        case STATE_WAIT_FOR_READ_ALL:
        {
            int c = d->socket->recv(reinterpret_cast<char*>(d->buff()), d->buffMaxSize(), 0);
            if (c > 0)
            {
                d->setBuffSize(static_cast<uint16_t>(c));
                d->state = STATE_OPENED;
                return Status_Good;
            }
            else if (c == 0)
            {
                this->close();
                // Note: When connection is remotely closed is not error for server side
                if (d->modeServer())
                    return Status_Uncertain;
                else
                    return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                          StringLiteral("'. Remote connection closed") );
            }
            else if (isNonBlocking() && (GetTickCount() - d->timestamp >= d->timeout())) // waiting timeout read first byte elapsed
            {
                this->close();
                return d->setError(Modbus::Status_BadTcpReadTimeout, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                                     StringLiteral("'. Timeout") );
            }
            else
            {
                int e = WSAGetLastError();
                if (isNonBlocking() && e == WSAEWOULDBLOCK)
                    return Status_Processing; // No data available for non-blocking socket, try again later
                this->close();
                return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                      StringLiteral("'. Error code: ") + toModbusString(e) +
                                                      StringLiteral(". ") + getLastErrorText());
            }
        }
            break;
        default:
            if (this->isOpen())
            {
                d->state = STATE_OPENED;
                fRepeatAgain = true;
            }
            else
                return d->setError(Status_BadTcpRead, StringLiteral("Internal state error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}
