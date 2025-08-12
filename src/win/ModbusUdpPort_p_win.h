#ifndef MODBUSUDPPORT_P_WIN_H
#define MODBUSUDPPORT_P_WIN_H

#include "../ModbusUdpPort_p.h"

#include "Modbus_win.h"

class ModbusUdpPortPrivateWin : public ModbusUdpPortPrivate
{
public:
    ModbusUdpPortPrivateWin(bool blocking) :
        ModbusUdpPortPrivate(blocking)
    {
        WSADATA data;
        WSAStartup(0x202, &data);

        this->timestamp = 0;
        this->socket = new ModbusSocket();
    }

    ~ModbusUdpPortPrivateWin()
    {
        if (!this->socket->isInvalid())
        {
            this->socket->shutdown();
            this->socket->close();
        }
        WSACleanup();
    }

public:
    inline sockaddr_in* p_sockaddr_in() { return &sockadr; }
    inline sockaddr* p_sockaddr() { return reinterpret_cast<sockaddr*>(p_sockaddr_in()); }

public:
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

public:
    ModbusSocket *socket;
    DWORD timestamp;
    sockaddr_in sockadr;
};

Handle ModbusUdpPortPrivateWin::handle() const
{
    return reinterpret_cast<Handle>(this->socket->socket());
}

StatusCode ModbusUdpPortPrivateWin::open()
{
    ModbusUdpPortPrivateWin *d = this;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_BEGIN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
            d->clearChanged();
            if (isOpen())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            if (d->modeServer)
            {
                d->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (d->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                sockadr.sin_family = AF_INET;
                sockadr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any available interface
                sockadr.sin_port = htons(d->port()); // Port number

                if (d->socket->bind(p_sockaddr(), sizeof(sockaddr_in)) == SOCKET_ERROR)
                {
                    d->socket->close();
                    d->state = STATE_CLOSED;
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpBind, (StringLiteral("UDP. Bind error for port '") + toModbusString(d->port()) +
                                                           StringLiteral("'. Error code: ") + toModbusString(err) +
                                                           StringLiteral(". ") + getLastErrorText()).data());
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
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while getting address info for '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(status) +
                                                            StringLiteral(". ") + getLastErrorText());
                memcpy(p_sockaddr(), addr->ai_addr ,sizeof(sockaddr));
                freeaddrinfo(addr);
                d->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (d->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return d->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                d->p_sockaddr_in()->sin_port = htons(d->port());
            }
            d->socket->setBlocking(isBlocking());
            if (isBlocking())
                d->socket->setTimeout(d->timeout());
            d->state = STATE_BEGIN;
            return Status_Good;
        default:
            if (!isOpen())
            {
                d->state = STATE_CLOSED;
                fRepeatAgain = true;
                break;
            }
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusUdpPortPrivateWin::close()
{
    ModbusUdpPortPrivateWin *d = this;
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusUdpPortPrivateWin::isOpen() const
{
    const ModbusUdpPortPrivateWin *d = this;
    if (d->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = d->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode ModbusUdpPortPrivateWin::write()
{
    ModbusUdpPortPrivateWin *d = this;
    switch (d->state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        int c = sendto(d->socket->socket(),
                       reinterpret_cast<char*>(d->buff),
                       d->sz,
                       0,
                       d->p_sockaddr(),
                       sizeof(sockaddr));
        if (c > 0)
        {
            d->state = STATE_BEGIN;
            return Status_Good;
        }
        else
        {
            d->close();
            DWORD err = WSAGetLastError();
            return d->setError(Status_BadUdpWrite, StringLiteral("UDP. Error while writing to '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
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

StatusCode ModbusUdpPortPrivateWin::read()
{
    ModbusUdpPortPrivateWin *d = this;
    const uint16_t size = d->c_buffSz;
    switch (d->state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_READ:
        d->timestamp = GetTickCount();
        d->state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        int addrsz = sizeof(sockaddr);
        int c = recvfrom(d->socket->socket(),
                         reinterpret_cast<char*>(d->buff),
                         size,
                         0,
                         d->p_sockaddr(),
                         &addrsz);
        if (c > 0)
        {
            d->sz = static_cast<uint16_t>(c);
            d->state = STATE_BEGIN;
            return Status_Good;
        }
        else if (c == 0)
        {
            close();
            // Note: When connection is remotely closed is not error for server side
            if (d->modeServer)
                return Status_Uncertain;
            else
                return d->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
                                                      StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (GetTickCount() - d->timestamp >= this->settings.timeout)) // waiting timeout read first byte elapsed
        {
            close();
            return d->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
                                                  StringLiteral("'. Timeout") );
        }
        else
        {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                close();
                return d->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + d->settings.hostOrPortName + StringLiteral(":") + toModbusString(d->settings.port) +
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

inline ModbusUdpPortPrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusUdpPortPrivateWin*>(d_ptr); }

#endif // MODBUSUDPPORT_P_WIN_H
