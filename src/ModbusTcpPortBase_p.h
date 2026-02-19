#ifndef MODBUSTCPPORTBASE_P_H
#define MODBUSTCPPORTBASE_P_H

#include "ModbusNetPort_p.h"

class ModbusTcpPortBasePrivate : public ModbusNetPortPrivate
{
public:
    static ModbusTcpPortBasePrivate *create(ModbusFramePrivate *f, ModbusSocket *socket, bool blocking);

public:
    using ModbusNetPortPrivate::ModbusNetPortPrivate;
};

#endif // MODBUSTCPPORTBASE_P_H