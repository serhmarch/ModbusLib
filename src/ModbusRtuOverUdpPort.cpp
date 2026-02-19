#include "ModbusRtuOverUdpPort.h"
#include "ModbusUdpPortBase_p.h"
#include "ModbusRtuFrame_p.h"

ModbusRtuOverUdpPort::ModbusRtuOverUdpPort(bool blocking) :
    ModbusUdpPortBase(ModbusUdpPortBasePrivate::create(new ModbusRtuFramePrivate(), blocking))
{
}
