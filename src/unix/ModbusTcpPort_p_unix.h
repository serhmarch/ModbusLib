#ifndef MODBUSTCPPORT_P_UNIX_H
#define MODBUSTCPPORT_P_UNIX_H

#include <netdb.h>

#include "../ModbusTcpPort_p.h"

#include "ModbusTCP_unix.h"

class ModbusTcpPortPrivateUnix : public ModbusTcpPortPrivate
{
public:
    ModbusTcpPortPrivateUnix(ModbusTcpSocket *socket, bool blocking) :
        ModbusTcpPortPrivate(blocking)
    {
        this->timestamp = 0;
        this->addr = nullptr;

        if (socket)
        {
            socket->setBlocking(isBlocking());
            this->socket = socket;
        }
        else
        {
            this->socket = new ModbusTcpSocket();
        }
    }

    ~ModbusTcpPortPrivateUnix()
    {
        if (!this->socket->isInvalid())
        {
            this->socket->shutdown();
            this->socket->close();
        }
        this->freeAddr();
    }

public:
    inline void freeAddr()
    {
        if (this->addr)
        {
            freeaddrinfo(addr);
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
    ModbusTcpSocket *socket;
    Timer timestamp;
    struct addrinfo *addr;
};

Handle ModbusTcpPortPrivateUnix::handle() const
{
    return reinterpret_cast<Handle>(this->socket->socket());
}

StatusCode ModbusTcpPortPrivateUnix::open()
{
    ModbusTcpPortPrivateUnix *d = this;
    bool fRepeatAgain;
    do
    {
        fRepeatAgain = false;
        switch (d->state)
        {
        case STATE_BEGIN:
        case STATE_CLOSED:
        {
            d->clearChanged();
            if (isOpen())
            {
                d->state = STATE_OPENED;
                return Status_Good;
            }
            struct addrinfo hints;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            d->freeAddr();

            struct addrinfo* addr = nullptr;
            int status = getaddrinfo(d->host().data(), nullptr, &hints, &addr);
            if (status != 0)
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info for '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                        StringLiteral(". ") + getLastErrorText());
            d->addr = addr;
            d->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (d->socket->isInvalid())
            {
                d->freeAddr();
                return d->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket for '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                        StringLiteral(". ") + getLastErrorText());
            }
            d->socket->setBlocking(false); // Note: in case of block-socket it will be set after connect
            if (isBlocking())
                d->socket->setTimeout(this->timeout());
            reinterpret_cast<sockaddr_in*>(d->addr->ai_addr)->sin_port = htons(d->settings.port);
            d->timestamp = timer();
            d->state = STATE_WAIT_FOR_OPEN;
        }
            // no need break
        case STATE_WAIT_FOR_OPEN:
        {
            if (isNonBlocking())
            {
                int r = d->socket->connect(d->addr->ai_addr, static_cast<int>(d->addr->ai_addrlen));
                if ((r == 0) || (errno == EISCONN))
                {
                    d->state = STATE_BEGIN;
                    return Status_Good;
                }
                else if (timer() - d->timestamp >= this->timeout())
                {
                    d->socket->close();
                    d->state = STATE_CLOSED;
                    return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                            StringLiteral("'. Timeout") );
                }
            }
            else
            {
                int r = d->socket->connect(d->addr->ai_addr, static_cast<int>(d->addr->ai_addrlen));
                if (r != 0 && errno != EISCONN)
                {
                    if (errno != EINPROGRESS)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                    // Use select() to wait for writability (i.e., successful connect)
                    fd_set writefds;
                    FD_ZERO(&writefds);
                    FD_SET(d->socket->socket(), &writefds);

                    struct timeval tv;
                    tv.tv_sec = this->timeout() / 1000;
                    tv.tv_usec = (this->timeout() % 1000) * 1000;
                    
                    // From Linux man: select(int nfds, ...)
                    // nfds - This argument should be set to the highest-numbered file descriptor in any of the three sets, plus 1. 
                    //        The indicated file descriptors in each set are checked, up to this limit
                    r = select(d->socket->socket()+1, nullptr, &writefds, nullptr, &tv);
                    if (r <= 0)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        if (r == 0)
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                    StringLiteral("'. Timeout") );
                        else
                            return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                    StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                                    StringLiteral(". ") + getLastErrorText());
                    }

                    // Check for errors after select
                    int sockErr = 0;
                    socklen_t len = sizeof(sockErr);
                    getsockopt(d->socket->socket(), SOL_SOCKET, SO_ERROR, (char*)&sockErr, &len);
                    if (sockErr != 0)
                    {
                        d->socket->close();
                        d->state = STATE_CLOSED;
                        return d->setError(Status_BadTcpConnect,StringLiteral("TCP. Error while connecting to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                                StringLiteral("'. Error code: ") + toModbusString(sockErr) +
                                                                StringLiteral(". ") + getLastErrorText());
                    }
                }
                // Success - set blocking mode
                d->socket->setBlocking(true);
                d->state = STATE_BEGIN;
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
            return Status_Good;
        }
    }
    while (fRepeatAgain);
    return Status_Processing;
}

StatusCode ModbusTcpPortPrivateUnix::close()
{
    ModbusTcpPortPrivateUnix *d = this;
    if (!d->socket->isInvalid())
    {
        d->socket->shutdown();
        d->socket->close();
    }
    d->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusTcpPortPrivateUnix::isOpen() const
{
    const ModbusTcpPortPrivateUnix *d = this;
    if (d->socket->isInvalid())
        return false;
    int error = 0;
    socklen_t error_len = sizeof(error);
    int r = d->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode ModbusTcpPortPrivateUnix::write()
{
    ModbusTcpPortPrivateUnix *d = this;
    switch (d->state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        ssize_t c = d->socket->send(reinterpret_cast<char*>(d->buff), d->sz, 0);
        if (c > 0)
        {
            d->state = STATE_BEGIN;
            return Status_Good;
        }
        else
        {
            close();
            return d->setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing to '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                   StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                   StringLiteral(". ") + getLastErrorText());
        }
    }
        break;
    default:
        break;
    }
    return Status_Processing;
}

StatusCode ModbusTcpPortPrivateUnix::read()
{
    ModbusTcpPortPrivateUnix *d = this;
    const uint16_t size = MBCLIENTTCP_BUFF_SZ;
    switch (d->state)
    {
    case STATE_BEGIN:
    case STATE_PREPARE_TO_READ:
        d->timestamp = timer();
        d->state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        ssize_t c = d->socket->recv(reinterpret_cast<char*>(d->buff), size, 0);
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
                return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                    StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (timer() - d->timestamp >= d->timeout())) // waiting timeout read first byte elapsed
        {
            close();
            return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                  StringLiteral("'. Timeout") );
        }
        else
        {
            int e = errno;
            if (e != EWOULDBLOCK)
            {
                close();
                return d->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + d->host() + StringLiteral(":") + toModbusString(d->settings.port) +
                                                      StringLiteral("'. Error code: ") + toModbusString(errno) +
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

inline ModbusTcpPortPrivateUnix *d_unix(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortPrivateUnix*>(d_ptr); }

#endif // MODBUSTCPPORT_P_UNIX_H
