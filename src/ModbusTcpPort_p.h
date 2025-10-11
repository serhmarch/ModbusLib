#ifndef MODBUSTCPPORT_P_H
#define MODBUSTCPPORT_P_H

#include "ModbusPort_p.h"
#include "ModbusTcpPort.h"

#define MBCLIENTTCP_BUFF_SZ MB_TCP_IO_BUFF_SZ

class ModbusTcpPortPrivate : public ModbusPortPrivate
{
public:
    static ModbusTcpPortPrivate *create(ModbusSocket *socket, bool blocking);

public:
    ModbusTcpPortPrivate(bool blocking) :
        ModbusPortPrivate(MBCLIENTTCP_BUFF_SZ, blocking)
    {
        const Modbus::NetDefaults &d = Modbus::NetDefaults::instance();

        setHost   (d.host   );
        setPort   (d.port   );
        setTimeout(d.timeout);

        autoIncrement = true;
        transaction = 0;
    }

public:
    void setNextRequestRepeated(bool v) override { autoIncrement = !v; }

public:
    bool autoIncrement;
    uint16_t transaction;
};

inline ModbusTcpPortPrivate *d_ModbusTcpPort(ModbusPortPrivate *d_ptr) { return static_cast<ModbusTcpPortPrivate*>(d_ptr); }

#endif // MODBUSTCPPORT_P_H