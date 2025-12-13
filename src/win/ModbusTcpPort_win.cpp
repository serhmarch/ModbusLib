#include "../ModbusTcpPort.h"

#include "ModbusTcpPort_p_win.h"

ModbusTcpPortPrivate *ModbusTcpPortPrivate::create(ModbusSocket *socket, bool blocking)
{
    return new ModbusTcpPortPrivateWin(socket, blocking);
}
