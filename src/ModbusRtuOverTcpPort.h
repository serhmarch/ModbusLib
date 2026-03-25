/*!
 * \file   ModbusRtuOverTcpPort.h
 * \brief  Contains definition of RTU over TCP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSRTUOVERTCPPORT_H
#define MODBUSRTUOVERTCPPORT_H

#include "ModbusTcpPortBase.h"

class ModbusSocket;

/*! \brief Implements RTU over TCP version of the Modbus communication protocol.

    \details `ModbusRtuOverTcpPort` derived from `ModbusTcpPortBase` and implements RTU framing
    for Modbus communication over TCP. This protocol combines the compact binary encoding of
    Modbus RTU with the network transport capabilities of TCP, providing efficient data transfer
    while maintaining network connectivity.

    The RTU variant of Modbus encodes protocol data in binary form and includes CRC-based integrity
    checking, making it suitable for bandwidth-efficient communication and interoperability with
    devices that rely on RTU framing. When combined with TCP, this class enables RTU-framed Modbus
    protocol transmission over TCP/IP networks, facilitating integration with systems that require
    RTU-compatible payloads across network infrastructure.

    This implementation supports both client-side and server-side communication modes. When used in
    server mode, an existing socket connection can be provided during instantiation. The class
    handles frame boundaries and validation for RTU-formatted messages, ensuring proper message
    framing and integrity across TCP streams. Both blocking and non-blocking socket modes are
    supported to accommodate different application requirements and I/O patterns.

 */
class MODBUS_EXPORT ModbusRtuOverTcpPort : public ModbusTcpPortBase
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
    using ModbusTcpPortBase::ModbusTcpPortBase;
};

#endif // MODBUSRTUOVERTCPPORT_H
