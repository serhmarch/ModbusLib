#include "../ModbusUdpPort.h"

#include "ModbusUdpPort_p_unix.h"

ModbusUdpPortPrivate *ModbusUdpPortPrivate::create(bool blocking)
{
    return new ModbusUdpPortPrivateUnix(blocking);
}

