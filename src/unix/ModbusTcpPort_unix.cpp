#include "../ModbusTcpPort.h"

#include "ModbusTcpPort_p_unix.h"

ModbusTcpPortPrivate *ModbusTcpPortPrivate::create(ModbusSocket *socket, bool blocking)
{
    return new ModbusTcpPortPrivateUnix(socket, blocking);
}

