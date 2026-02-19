#ifndef MODBUSTCPPORTBASE_P_UNIX_H
#define MODBUSTCPPORTBASE_P_UNIX_H

#include <netdb.h>

#include "../ModbusTcpPortBase_p.h"

#include "Modbus_unix.h"

class ModbusTcpPortBasePrivateUnix : public ModbusTcpPortBasePrivate
{
public:
    ModbusTcpPortBasePrivateUnix(ModbusFramePrivate *f, ModbusSocket *socket, bool blocking) :
        ModbusTcpPortBasePrivate(f, blocking)
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

    ~ModbusTcpPortBasePrivateUnix()
    {
        if (this->socket->isValid())
        {
            this->socket->shutdown();
            this->socket->close();
        }
        this->freeAddr();
        delete this->socket;
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
    ModbusSocket *socket;
    Timer timestamp;
    struct addrinfo *addr;
};

inline ModbusTcpPortBasePrivateUnix *d_unix(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortBasePrivateUnix*>(d_ptr); }

#endif // MODBUSTCPPORTBASE_P_UNIX_H
