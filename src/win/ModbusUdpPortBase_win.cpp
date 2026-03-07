#include "../ModbusUdpPort.h"

#include "ModbusUdpPortBase_p_win.h"

ModbusUdpPortBasePrivate *ModbusUdpPortBasePrivate::create(ModbusFramePrivate *f, bool blocking)
{
    return new ModbusUdpPortBasePrivateWin(f, blocking);
}

Modbus::Handle ModbusUdpPortBase::handle() const
{
    return reinterpret_cast<Handle>(d_win(d_ptr)->socket->socket());
}

Modbus::StatusCode ModbusUdpPortBase::open()
{
    ModbusUdpPortBasePrivateWin *d = d_win(d_ptr);
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
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
            if (d->modeServer())
            {
                d->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (d->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                d->sockadr.sin_family = AF_INET;
                d->sockadr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any available interface
                d->sockadr.sin_port = htons(d->port()); // Port number

                if (d->socket->bind(d->p_sockaddr(), sizeof(sockaddr_in)) == SOCKET_ERROR)
                {
                    d->socket->close();
                    d->state = STATE_CLOSED;
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpBind, StringLiteral("UDP. Bind error for port '") + toModbusString(d->port()) +
                                                          StringLiteral("'. Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
                }
            }
            else
            {
                ADDRINFO hints;
                ZeroMemory(&hints, sizeof(hints));
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;

                ADDRINFO* addr = nullptr;
                DWORD status = getaddrinfo(d->host().data(), NULL, &hints, &addr);
                if (status != 0)
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while getting address info for '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                            StringLiteral("'. Error code: ") + toModbusString(status) +
                                                            StringLiteral(". ") + getLastErrorText());
                memcpy(d->p_sockaddr(), addr->ai_addr ,sizeof(sockaddr));
                freeaddrinfo(addr);
                d->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (d->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                d->p_sockaddr_in()->sin_port = htons(d->port());
            }
            d->socket->setBlocking(d->isBlocking());
            if (d->isBlocking())
                d->socket->setTimeout(d->timeout());
            d->state = STATE_OPENED;
            return Status_Good;
        default:
            if (this->isOpen() && !d->isChanged())
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

Modbus::StatusCode ModbusUdpPortBase::close()
{
    ModbusUdpPortBasePrivateWin *d = d_win(d_ptr);
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusUdpPortBase::isOpen() const
{
    ModbusUdpPortBasePrivateWin *d = d_win(d_ptr);
    if (d->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = d->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

Modbus::StatusCode ModbusUdpPortBase::write()
{
    ModbusUdpPortBasePrivateWin *d = d_win(d_ptr);
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
            int c = sendto(d->socket->socket(),
                        reinterpret_cast<char*>(d->buff()),
                        d->buffSize(),
                        0,
                        d->p_sockaddr(),
                        sizeof(sockaddr));
            if (c > 0)
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            else
            {
                this->close();
                DWORD err = WSAGetLastError();
                return d->setError(Status_BadUdpWrite, StringLiteral("UDP. Error while writing to '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
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
                return d->setError(Status_BadUdpWrite, StringLiteral("Internal state error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

Modbus::StatusCode ModbusUdpPortBase::read()
{
    ModbusUdpPortBasePrivateWin *d = d_win(d_ptr);
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
            int addrsz = sizeof(sockaddr);
            int c = recvfrom(d->socket->socket(),
                            reinterpret_cast<char*>(d->buff()),
                            d->buffMaxSize(),
                            0,
                            d->p_sockaddr(),
                            &addrsz);
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
                    return d->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                          StringLiteral("'. Remote connection closed") );
            }
            else if (isNonBlocking() && (GetTickCount() - d->timestamp >= d->timeout())) // waiting timeout to read first byte was elapsed
            {
                //this->close();
                d->state = STATE_OPENED;
                return d->setError(Status_BadUdpReadTimeout, StringLiteral("UDP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                             StringLiteral("'. Timeout") );
            }
            else
            {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK)
                {
                    this->close();
                    return d->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->port()) +
                                                          StringLiteral("'. Error code: ") + toModbusString(err) +
                                                          StringLiteral(". ") + getLastErrorText());
                }
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
                return d->setError(Status_BadUdpRead, StringLiteral("Internal state error"));
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

