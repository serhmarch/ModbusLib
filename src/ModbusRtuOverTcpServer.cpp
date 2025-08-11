#include "ModbusRtuOverTcpServer.h"
#include "ModbusRtuOverTcpPort.h"

ModbusPort *ModbusRtuOverTcpServer::createModbusPort(ModbusTcpSocket *socket)
{
    return new ModbusRtuOverTcpPort(socket);
}
