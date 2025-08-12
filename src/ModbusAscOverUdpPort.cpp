#include "ModbusAscOverUdpPort.h"
#include "ModbusUdpPort_p.h"

ModbusAscOverUdpPort::ModbusAscOverUdpPort(bool blocking) :
    ModbusAscPort(ModbusUdpPortPrivate::create(blocking))
{
}
