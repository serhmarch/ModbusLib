#include "ModbusUdpPort.h"
#include "ModbusUdpPort_p.h"

ModbusUdpPort::ModbusUdpPort(bool blocking) :
    ModbusTcpPort(ModbusUdpPortPrivate::create(blocking))
{
}
