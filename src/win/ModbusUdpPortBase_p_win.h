#ifndef MODBUSUDPPORTBASE_P_WIN_H
#define MODBUSUDPPORTBASE_P_WIN_H

#include "../ModbusUdpPortBase_p.h"

#include "Modbus_win.h"

class ModbusUdpPortBasePrivateWin : public ModbusUdpPortBasePrivate
{
public:
    ModbusUdpPortBasePrivateWin(ModbusFramePrivate *f, bool blocking) :
        ModbusUdpPortBasePrivate(f, blocking)
    {
        WSADATA data;
        WSAStartup(0x202, &data);

        this->timestamp = 0;
        this->socket = new ModbusSocket();
    }

    ~ModbusUdpPortBasePrivateWin()
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
    ModbusSocket *socket;
    DWORD timestamp;
    sockaddr_in sockadr;
};

inline ModbusUdpPortBasePrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusUdpPortBasePrivateWin*>(d_ptr); }

#endif // MODBUSUDPPORTBASE_P_WIN_H
