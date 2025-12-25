/*!
 * \file   ModbusAscOverTcpPort.h
 * \brief  Contains definition of ASC over TCP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSASCOVERTCPPORT_H
#define MODBUSASCOVERTCPPORT_H

#include "ModbusAscPort.h"

class ModbusSocket;

/*! \brief Implements ASC over TCP version of the Modbus communication protocol.

    \details `ModbusAscPort` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for ASC version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusAscOverTcpPort : public ModbusAscPort
{
public:
    /// \details Constructor of the class.
    /// `socket` is a pointer to the previously created socket. It's useful for server-side class.
    /// if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscOverTcpPort(ModbusSocket *socket, bool blocking = false);

    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscOverTcpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscOverTcpPort` returns `Modbus::ASCvTCP`.
    Modbus::ProtocolType type() const override { return Modbus::ASCvTCP; }

protected:
    using ModbusAscPort::ModbusAscPort;
};

#endif // MODBUSASCOVERTCPPORT_H
