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
        delete this->socket;
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
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (this->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        case STATE_WAIT_FOR_OPEN:
            this->clearChanged();
            if (isOpen())
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            if (this->modeServer)
            {
                this->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (this->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return this->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                sockadr.sin_family = AF_INET;
                sockadr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any available interface
                sockadr.sin_port = htons(this->port()); // Port number

                if (this->socket->bind(p_sockaddr(), sizeof(sockaddr_in)) == SOCKET_ERROR)
                {
                    this->socket->close();
                    this->state = STATE_CLOSED;
                    int err = WSAGetLastError();
                    return this->setError(Status_BadUdpBind, (StringLiteral("UDP. Bind error for port '") + toModbusString(this->port()) +
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
                DWORD status = getaddrinfo(this->host().data(), NULL, &hints, &addr);
                if (status != 0)
                    return this->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while getting address info for '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(status) +
                                                            StringLiteral(". ") + getLastErrorText());
                memcpy(p_sockaddr(), addr->ai_addr ,sizeof(sockaddr));
                freeaddrinfo(addr);
                this->socket->create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (this->socket->isInvalid())
                {
                    int err = WSAGetLastError();
                    return this->setError(Status_BadUdpCreate, StringLiteral("UDP. Error while creating socket for '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                            StringLiteral("'. Error code: ") + toModbusString(err) +
                                                            StringLiteral(". ") + getLastErrorText());
                }
                this->p_sockaddr_in()->sin_port = htons(this->port());
            }
            this->socket->setBlocking(isBlocking());
            if (isBlocking())
                this->socket->setTimeout(this->timeout());
            this->state = STATE_OPENED;
            return Status_Good;
        default:
            if (!isOpen())
            {
                this->state = STATE_CLOSED;
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
    if (!this->socket->isInvalid())
    {
        this->socket->shutdown();
        this->socket->close();
    }
    this->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusUdpPortPrivateWin::isOpen() const
{
    if (this->socket->isInvalid())
        return false;
    int error = 0;
    int error_len = sizeof(error);
    int r = this->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode ModbusUdpPortPrivateWin::write()
{
    switch (this->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        int c = sendto(this->socket->socket(),
                       reinterpret_cast<char*>(this->buff),
                       this->sz,
                       0,
                       this->p_sockaddr(),
                       sizeof(sockaddr));
        if (c > 0)
        {
            this->state = STATE_OPENED;
            return Status_Good;
        }
        else
        {
            this->close();
            DWORD err = WSAGetLastError();
            return this->setError(Status_BadUdpWrite, StringLiteral("UDP. Error while writing to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
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
    const uint16_t size = this->c_buffSz;
    switch (this->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_READ:
        this->timestamp = GetTickCount();
        this->state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        int addrsz = sizeof(sockaddr);
        int c = recvfrom(this->socket->socket(),
                         reinterpret_cast<char*>(this->buff),
                         size,
                         0,
                         this->p_sockaddr(),
                         &addrsz);
        if (c > 0)
        {
            this->sz = static_cast<uint16_t>(c);
            this->state = STATE_OPENED;
            return Status_Good;
        }
        else if (c == 0)
        {
            close();
            // Note: When connection is remotely closed is not error for server side
            if (this->modeServer)
                return Status_Uncertain;
            else
                return this->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                         StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (GetTickCount() - this->timestamp >= this->timeout())) // waiting timeout read first byte elapsed
        {
            //close();
            this->state = STATE_OPENED;
            return this->setError(Status_BadUdpReadTimeout, StringLiteral("UDP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                            StringLiteral("'. Timeout") );
        }
        else
        {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                close();
                return this->setError(Status_BadUdpRead, StringLiteral("UDP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
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
