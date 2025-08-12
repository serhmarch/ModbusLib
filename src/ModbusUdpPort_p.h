#ifndef MODBUSUDPPORT_P_H
#define MODBUSUDPPORT_P_H

#include "ModbusTcpPort_p.h"
#include "ModbusUdpPort.h"

#define MBCLIENTUDP_BUFF_SZ MB_TCP_IO_BUFF_SZ

class ModbusUdpPortPrivate : public ModbusTcpPortPrivate
{
public:
    static ModbusUdpPortPrivate *create(bool blocking);

public:
    ModbusUdpPortPrivate(bool blocking) :
        ModbusTcpPortPrivate(blocking)
    {
    }

};

inline ModbusUdpPortPrivate *d_ModbusUdpPort(ModbusPortPrivate *d_ptr) { return static_cast<ModbusUdpPortPrivate*>(d_ptr); }

#endif // MODBUSUDPPORT_P_H
