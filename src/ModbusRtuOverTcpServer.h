/*!
 * \file   ModbusRtuOverTcpServer.h
 * \brief  Header file of Modbus RTU over TCP server.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSRTUOVERTCPSERVER_H
#define MODBUSRTUOVERTCPSERVER_H

#include "ModbusTcpServer.h"

/*! \brief The `ModbusRtuOverTcpServer` class implements RTU over TCP server part of the Modbus protocol.

    \details `ModbusTcpServer` ...

 */

class MODBUS_EXPORT ModbusRtuOverTcpServer : public ModbusTcpServer
{
public:
    using ModbusTcpServer::ModbusTcpServer;

public:
    ModbusPort *createModbusPort(ModbusTcpSocket *socket) override;
};

#endif // MODBUSRTUOVERTCPSERVER_H
