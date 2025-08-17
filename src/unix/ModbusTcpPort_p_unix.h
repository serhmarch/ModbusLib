#ifndef MODBUSTCPPORT_P_UNIX_H
#define MODBUSTCPPORT_P_UNIX_H

#include <netdb.h>

#include "../ModbusTcpPort_p.h"

#include "Modbus_unix.h"

class ModbusTcpPortPrivateUnix : public ModbusTcpPortPrivate
{
public:
    ModbusTcpPortPrivateUnix(ModbusSocket *socket, bool blocking) :
        ModbusTcpPortPrivate(blocking)
    {
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
    ModbusSocket *socket;
    Timer timestamp;
    struct addrinfo *addr;
};

Handle ModbusTcpPortPrivateUnix::handle() const
{
    return reinterpret_cast<Handle>(this->socket->socket());
}

StatusCode ModbusTcpPortPrivateUnix::open()
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
            this->clearChanged();
            if (isOpen())
            {
                this->state = STATE_OPENED;
                return Status_Good;
            }
            struct addrinfo hints;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            this->freeAddr();

            struct addrinfo* addr = nullptr;
            int status = getaddrinfo(this->host().data(), nullptr, &hints, &addr);
            if (status != 0)
                return this->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while getting address info for '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                        StringLiteral(". ") + getLastErrorText());
            this->addr = addr;
            this->socket->create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (this->socket->isInvalid())
            {
                this->freeAddr();
                return this->setError(Status_BadTcpCreate, StringLiteral("TCP. Error while creating socket for '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                        StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                        StringLiteral(". ") + getLastErrorText());
            }
            this->socket->setBlocking(false); // Note: in case of block-socket it will be set after connect
            if (isBlocking())
                this->socket->setTimeout(this->timeout());
            reinterpret_cast<sockaddr_in*>(this->addr->ai_addr)->sin_port = htons(this->settings.port);
            this->timestamp = timer();
            this->state = STATE_WAIT_FOR_OPEN;
        }
            // no need break
        case STATE_WAIT_FOR_OPEN:
        {
            if (isNonBlocking())
            {
                int r = this->socket->connect(this->addr->ai_addr, static_cast<int>(this->addr->ai_addrlen));
                if ((r == 0) || (errno == EISCONN))
                {
                    this->state = STATE_OPENED;
                    return Status_Good;
                }
                else if (timer() - this->timestamp >= this->timeout())
                {
                    this->socket->close();
                    this->state = STATE_CLOSED;
                    return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                StringLiteral("'. Timeout") );
                }
            }
            else
            {
                int r = this->socket->connect(this->addr->ai_addr, static_cast<int>(this->addr->ai_addrlen));
                if (r != 0 && errno != EISCONN)
                {
                    if (errno != EINPROGRESS)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                    StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                                    StringLiteral(". ") + getLastErrorText());
                    }
                    // Use select() to wait for writability (i.e., successful connect)
                    fd_set writefds;
                    FD_ZERO(&writefds);
                    FD_SET(this->socket->socket(), &writefds);

                    struct timeval tv;
                    tv.tv_sec = this->timeout() / 1000;
                    tv.tv_usec = (this->timeout() % 1000) * 1000;
                    
                    // From Linux man: select(int nfds, ...)
                    // nfds - This argument should be set to the highest-numbered file descriptor in any of the three sets, plus 1. 
                    //        The indicated file descriptors in each set are checked, up to this limit
                    r = select(this->socket->socket()+1, nullptr, &writefds, nullptr, &tv);
                    if (r <= 0)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        if (r == 0)
                            return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                        StringLiteral("'. Timeout") );
                        else
                            return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                                        StringLiteral("'. Error code: ") + toModbusString(errno) +
                                                                        StringLiteral(". ") + getLastErrorText());
                    }

                    // Check for errors after select
                    int sockErr = 0;
                    socklen_t len = sizeof(sockErr);
                    getsockopt(this->socket->socket(), SOL_SOCKET, SO_ERROR, (char*)&sockErr, &len);
                    if (sockErr != 0)
                    {
                        this->socket->close();
                        this->state = STATE_CLOSED;
                        return this->setError(Status_BadTcpConnect, StringLiteral("TCP. Error while connecting to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
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

StatusCode ModbusTcpPortPrivateUnix::close()
{
    if (!this->socket->isInvalid())
    {
        this->socket->shutdown();
        this->socket->close();
    }
    this->state = STATE_CLOSED;
    return Status_Good;
}

bool ModbusTcpPortPrivateUnix::isOpen() const
{
    if (this->socket->isInvalid())
        return false;
    int error = 0;
    socklen_t error_len = sizeof(error);
    int r = this->socket->getsockopt(SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
    if (r != 0)
        return false;
    return (error == 0);
}

StatusCode ModbusTcpPortPrivateUnix::write()
{
    switch (this->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_WRITE:
    case STATE_WAIT_FOR_WRITE:
    case STATE_WAIT_FOR_WRITE_ALL:
    {
        ssize_t c = this->socket->send(reinterpret_cast<char*>(this->buff), this->sz, 0);
        if (c > 0)
        {
            this->state = STATE_OPENED;
            return Status_Good;
        }
        else
        {
            close();
            return this->setError(Status_BadTcpWrite, StringLiteral("TCP. Error while writing to '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
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
    const uint16_t size = MBCLIENTTCP_BUFF_SZ;
    switch (this->state)
    {
    case STATE_OPENED:
    case STATE_PREPARE_TO_READ:
        this->timestamp = timer();
        this->state = STATE_WAIT_FOR_READ;
        // no need break
    case STATE_WAIT_FOR_READ:
    case STATE_WAIT_FOR_READ_ALL:
    {
        ssize_t c = this->socket->recv(reinterpret_cast<char*>(this->buff), size, 0);
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
                return this->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                         StringLiteral("'. Remote connection closed") );
        }
        else if (isNonBlocking() && (timer() - this->timestamp >= this->timeout())) // waiting timeout read first byte elapsed
        {
            close();
            return this->setError(Status_BadTcpReadTimeout, StringLiteral("TCP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
                                                            StringLiteral("'. Timeout") );
        }
        else
        {
            int e = errno;
            if (e != EWOULDBLOCK)
            {
                close();
                return this->setError(Status_BadTcpRead, StringLiteral("TCP. Error while reading from '") + this->host() + StringLiteral(":") + toModbusString(this->settings.port) +
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
