#ifndef MODBUSTCPPORT_P_WIN_H
#define MODBUSTCPPORT_P_WIN_H

#include "../ModbusTcpPort_p.h"

#include "Modbus_win.h"

class ModbusTcpPortPrivateWin : public ModbusTcpPortPrivate
{
public:
    ModbusTcpPortPrivateWin(ModbusSocket *socket, bool blocking) :
        ModbusTcpPortPrivate(blocking)
    {
        WSADATA data;
        WSAStartup(0x202, &data);

        this->timestamp = 0;
        this->addr = nullptr;

        if (socket)
        {
            socket->setBlocking(isBlocking());
            this->socket = socket;
            if (socket->isValid())
                this->state = STATE_OPENED;
        }
        else
        {
            this->socket = new ModbusSocket();
        }
    }

    ~ModbusTcpPortPrivateWin()
    {
        if (this->socket->isValid())
        {
            this->socket->shutdown();
            this->socket->close();
        }
        this->freeAddr();
        delete this->socket;
        WSACleanup();
    }

public:
    inline void freeAddr()
    {
        if (this->addr)
        {
            freeaddrinfo(reinterpret_cast<ADDRINFO*>(this->addr));
            this->addr = nullptr;
        }
    }

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
    void *addr;
};

Handle ModbusTcpPortPrivateWin::handle() const
{
    return reinterpret_cast<Handle>(this->socket->socket());
}

StatusCode ModbusTcpPortPrivateWin::open()
{
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (this->state)
        {
        case STATE_UNKNOWN:
        case STATE_CLOSED:
        {
            if (isOpen())
            {
                if (this->isChanged())
                    this->close();
                else
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
            }
            this->clearChanged();
            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            this->freeAddr();

            ADDRINFO* addr = nullptr;
            DWORD status = getaddrinfo(this->settings.hostOrPortName.data(), NULL, &hints, &addr);
            if (status != 0)
                return this->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info for '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                           StringLiteral("'. Error code: ") + toModbusString(status) +
                                                           StringLiteral(". ") + getLastErrorText());
            this->addr = addr;
            this->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (this->socket->isInvalid())
            {
                this->freeAddr();
                int err = WSAGetLastError();
                return this->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket for '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                           StringLiteral("'. Error code: ") + toModbusString(err) +
                                                           StringLiteral(". ") + getLastErrorText());
            }
            this->socket->setBlocking(false); // Note: in case of block-socket it will be set after connect
            if (isBlocking())
                this->socket->setTimeout(this->settings.timeout);
            reinterpret_cast<sockaddr_in*>(reinterpret_cast<ADDRINFO*>(this->addr)->ai_addr)->sin_port = htons(this->settings.port);
            this->timestamp = GetTickCount();
            this->state = STATE_WAIT_FOR_OPEN;
        }
        // no need break
        case STATE_WAIT_FOR_OPEN:
        {
            if (isNonBlocking())
            {
                int r = this->socket->connect(reinterpret_cast<ADDRINFO*>(this->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(this->addr)->ai_addrlen));
                DWORD err = WSAGetLastError();
                if ((r == 0) || (err == WSAEISCONN))
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                else if (GetTickCount() - this->timestamp >= this->settings.timeout)
                {
                    this->socket->close();
                    this->state = STATE_CLOSED;
                    return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                StringLiteral("'. Timeout") );
                }
            }
            else if (isBlocking())
            {
                int r = this->socket->connect(reinterpret_cast<ADDRINFO*>(this->addr)->ai_addr, static_cast<int>(reinterpret_cast<ADDRINFO*>(this->addr)->ai_addrlen));
                if (r != 0)
                {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        return this->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(err) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                    // Use select() to wait for writability (i.e., successful connect)
                    fd_set writefds;
                    FD_ZERO(&writefds);
                    FD_SET(this->socket->socket(), &writefds);

                    TIMEVAL tv;
                    tv.tv_sec = this->settings.timeout / 1000;
                    tv.tv_usec = (this->settings.timeout % 1000) * 1000;

                    r = select(0, NULL, &writefds, NULL, &tv);
                    if (r <= 0)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        if (r == 0)
                            return this->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                    StringLiteral("'. Timeout") );
                        else
                        {
                            int err = WSAGetLastError();
                            return this->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                    StringLiteral("'. Error code: ") + toModbusString(err) +
                                                                    StringLiteral(". ") + getLastErrorText());
                        }
                    }

                    // Check for errors after select
                    int sockErr = 0;
                    socklen_t len = sizeof(sockErr);
                    getsockopt(this->socket->socket(), SOL_SOCKET, SO_ERROR, (char*)&sockErr, &len);
                    if (sockErr != 0)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(sockErr) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                }
                // Success - set blocking mode
                this->socket->setBlocking(true);
                this->state = STATE_OPENED;
                return Status_Good;
            }
        }
            break;
        default:
            if (isOpen() && !this->isChanged())
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            this->state = STATE_CLOSED;
            fRepeatAgain = true;
            break;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusTcpPortPrivateWin::close()
{
    if (!this->socket->isInvalid())
    {
        this->socket->shutdown();
        this->socket->close();
    }
    this->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusTcpPortPrivateWin::isOpen() const
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

StatusCode ModbusTcpPortPrivateWin::write()
{
    switch (this->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        int c = this->socket->send(reinterpret_cast<char*>(this->buff), this->sz, 0);
        if (c > 0)
        {
            this->state = STATE_OPENED;
            return Status_Good;
        }
        else
        {
            close();
            DWORD err = WSAGetLastError();
            return this->setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing to '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
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

StatusCode ModbusTcpPortPrivateWin::read()
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
        int c = this->socket->recv(reinterpret_cast<char*>(this->buff), size, 0);
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
                return this->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                         StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (GetTickCount() - this->timestamp >= this->settings.timeout)) // waiting timeout read first byte elapsed
        {
            close();
            return this->setError(Modbus::Status_BadTcpReadTimeout, StringLiteral("TCP. Error while reading from '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                    StringLiteral("'. Timeout") );
        }
        else
        {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                close();
                return this->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + this->settings.hostOrPortName + StringLiteral(":") + toModbusString(this->settings.port) +
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

inline ModbusTcpPortPrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortPrivateWin*>(d_ptr); }

#endif // MODBUSTCPPORT_P_WIN_H
