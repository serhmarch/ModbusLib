/*!
 * \file   ModbusRtuOverTcpPort.h
 * \brief  Contains definition of RTU over TCP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSRTUOVERTCPPORT_H
#define MODBUSRTUOVERTCPPORT_H

#include "ModbusRtuPort.h"

class ModbusSocket;

/*! \brief Implements RTU over TCP version of the Modbus communication protocol.

    \details `ModbusRtuPort` derived from `ModbusSerialPort` and implements `writeBuffer` and `readBuffer`
    for RTU version of Modbus communication protocol.

 */
class MODBUS_EXPORT ModbusRtuOverTcpPort : public ModbusRtuPort
{
public:
    /// \details Constructor of the class.
    /// `socket` is a pointer to the previously created socket. It's useful for server-side class.
    /// if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuOverTcpPort(ModbusSocket *socket, bool blocking = false);

    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuOverTcpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusRtuOverTcpPort` returns `Modbus::RTUvTCP`.
    Modbus::ProtocolType type() const override { return Modbus::RTUvTCP; }

protected:
    using ModbusRtuPort::ModbusRtuPort;
};

#endif // MODBUSRTUOVERTCPPORT_H
