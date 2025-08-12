#include "../ModbusUdpPort.h"

#include "ModbusUdpPort_p_win.h"

ModbusUdpPortPrivate *ModbusUdpPortPrivate::create(bool blocking)
{
    return new ModbusUdpPortPrivateWin(blocking);
}

