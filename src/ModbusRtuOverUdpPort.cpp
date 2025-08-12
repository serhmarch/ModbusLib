#include "ModbusRtuOverUdpPort.h"
#include "ModbusUdpPort_p.h"

ModbusRtuOverUdpPort::ModbusRtuOverUdpPort(bool blocking) :
    ModbusRtuPort(ModbusUdpPortPrivate::create(blocking))
{
}
