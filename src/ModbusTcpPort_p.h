#ifndef MODBUSTCPPORT_P_H
#define MODBUSTCPPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusTcpPort.h"

#define MBCLIENTTCP_BUFF_SZ MB_TCP_IO_BUFF_SZ

class ModbusTcpPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusTcpPortPrivate *create(ModbusTcpSocket *socket, bool blocking);

public:
    ModbusTcpPortPrivate(bool blocking) :
        ModbusPortPrivate(MBCLIENTTCP_BUFF_SZ, blocking)
    {
        const Modbus::NetworkDefaults &d = Modbus::NetworkDefaults::instance();

        settings.hostOrPortName = d.host   ;
        settings.port           = d.port   ;
        settings.timeout        = d.timeout;

        autoIncrement = true;
        transaction = 0;
    }

public:
    void setNextRequestRepeated(bool v) override { autoIncrement = !v; }

public:
    inline auto host   () { return settings.hostOrPortName; }
    inline auto port   () { return settings.port          ; }
    inline auto timeout() { return settings.timeout       ; }
       
public:
    bool autoIncrement;
    uint16_t transaction;
};

inline ModbusTcpPortPrivate *d_ModbusTcpPort(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortPrivate*>(d_ptr); }

#endif // MODBUSTCPPORT_P_H
