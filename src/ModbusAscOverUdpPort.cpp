#include "ModbusAscOverUdpPort.h"
#include "ModbusUdpPortBase_p.h"
#include "ModbusAscFrame_p.h"

ModbusAscOverUdpPort::ModbusAscOverUdpPort(bool blocking) :
    ModbusUdpPortBase(ModbusUdpPortBasePrivate::create(new ModbusAscFramePrivate(), blocking))
{
}
