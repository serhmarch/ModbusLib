#ifndef MODBUSUDPPORTBASE_P_UNIX_H
#define MODBUSUDPPORTBASE_P_UNIX_H

#include <netdb.h>

#include "../ModbusUdpPortBase_p.h"

#include "Modbus_unix.h"

class ModbusUdpPortBasePrivateUnix : public ModbusUdpPortBasePrivate
{
public:
    ModbusUdpPortBasePrivateUnix(ModbusFramePrivate *f, bool blocking) :
        ModbusUdpPortBasePrivate(f, blocking)
    {
        this->timestamp = 0;
        this->socket = new ModbusSocket();
    }

    ~ModbusUdpPortBasePrivateUnix()
    {
        if (!this->socket->isInvalid())
        {
            this->socket->shutdown();
            this->socket->close();
        }
        delete this->socket;
    }

public:
    inline sockaddr_in* p_sockaddr_in() { return &sockadr; }
    inline sockaddr* p_sockaddr() { return reinterpret_cast<sockaddr*>(p_sockaddr_in()); }

public:
    ModbusSocket *socket;
    Timer timestamp;
    sockaddr_in sockadr;
};

inline ModbusUdpPortBasePrivateUnix *d_unix(ModbusPortPrivate *d_ptr) { return static_cast<ModbusUdpPortBasePrivateUnix*>(d_ptr); }

#endif // MODBUSUDPPORTBASE_P_UNIX_H
