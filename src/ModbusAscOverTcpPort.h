/*!
 * \file   ModbusAscOverTcpPort.h
 * \brief  Contains definition of ASC over TCP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSASCOVERTCPPORT_H
#define MODBUSASCOVERTCPPORT_H

#include "ModbusTcpPortBase.h"

class ModbusSocket;

/*! \brief Implements ASC over TCP version of the Modbus communication protocol.

    \details `ModbusAscOverTcpPort` derived from `ModbusTcpPortBase` and implements ASCII framing
    for Modbus communication over TCP. This protocol combines the human-readable ASCII encoding of
    Modbus with the network transport capabilities of TCP, allowing for easier debugging and
    monitoring while maintaining network connectivity.

    The ASCII variant of Modbus encodes all data as printable ASCII characters, making it suitable
    for serial communication lines and network protocols where binary data may be problematic. When
    combined with TCP, this class enables ASCII-encoded Modbus protocol transmission over TCP/IP
    networks, facilitating integration with systems that require human-readable protocol frames.

    This implementation supports both client-side and server-side communication modes. When used in
    server mode, an existing socket connection can be provided during instantiation. The class
    handles frame delimitation using ASCII-specific markers (colons for start-of-frame and line
    terminators for end-of-frame), ensuring proper message framing and integrity across TCP streams.
    Both blocking and non-blocking socket modes are supported to accommodate different application
    requirements and I/O patterns.

 */
class MODBUS_EXPORT ModbusAscOverTcpPort : public ModbusTcpPortBase
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
    using ModbusTcpPortBase::ModbusTcpPortBase;
};

#endif // MODBUSASCOVERTCPPORT_H
