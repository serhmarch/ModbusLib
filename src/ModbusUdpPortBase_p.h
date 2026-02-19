#ifndef MODBUSUDPPORTBASE_P_H
#define MODBUSUDPPORTBASE_P_H

#include "ModbusNetPort_p.h"

class ModbusUdpPortBasePrivate : public ModbusNetPortPrivate
{
public:
    static ModbusUdpPortBasePrivate *create(ModbusFramePrivate *f, bool blocking);

public:
    using ModbusNetPortPrivate::ModbusNetPortPrivate;
};

#endif // MODBUSUDPPORTBASE_P_H