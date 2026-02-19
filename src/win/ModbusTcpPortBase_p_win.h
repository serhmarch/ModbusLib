#ifndef MODBUSTCPPORT_P_WIN_H
#define MODBUSTCPPORT_P_WIN_H

#include "../ModbusTcpPortBase_p.h"

#include "Modbus_win.h"

class ModbusTcpPortBasePrivateWin : public ModbusTcpPortBasePrivate
{
public:
    ModbusTcpPortBasePrivateWin(ModbusFramePrivate *f, ModbusSocket *socket, bool blocking) :
        ModbusTcpPortBasePrivate(f, blocking)
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

    ~ModbusTcpPortBasePrivateWin()
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
    ModbusSocket *socket;
    DWORD timestamp;
    void *addr;
};

inline ModbusTcpPortBasePrivateWin *d_win(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortBasePrivateWin*>(d_ptr); }

#endif // MODBUSTCPPORT_P_WIN_H
