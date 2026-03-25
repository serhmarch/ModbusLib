/*!
 * \file   ModbusAscOverUdpPort.h
 * \brief  Contains definition of ASC over UDP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSASCOVERUDPPORT_H
#define MODBUSASCOVERUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Implements ASC over UDP version of the Modbus communication protocol.

    \details `ModbusAscOverUdpPort` derived from `ModbusUdpPortBase` and implements ASCII framing
    for Modbus communication over UDP. This protocol combines the human-readable ASCII encoding of
    Modbus with the network transport capabilities of UDP, allowing for easier debugging and
    monitoring while maintaining network connectivity.

    The ASCII variant of Modbus encodes all data as printable ASCII characters, making it suitable
    for serial communication lines and network protocols where binary data may be problematic. When
    combined with UDP, this class enables ASCII-encoded Modbus protocol transmission over UDP/IP
    networks, facilitating integration with systems that require human-readable protocol frames.

    This implementation supports connectionless communication over UDP datagrams. The class
    handles frame delimitation using ASCII-specific markers (colons for start-of-frame and line
    terminators for end-of-frame), ensuring proper message framing and integrity across UDP packets.
    Both blocking and non-blocking socket modes are supported to accommodate different application
    requirements and I/O patterns.

 */
class MODBUS_EXPORT ModbusAscOverUdpPort : public ModbusUdpPortBase
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusAscOverUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusAscOverUdpPort` returns `Modbus::ASCvUDP`.
    Modbus::ProtocolType type() const override { return Modbus::ASCvUDP; }

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSASCOVERUDPPORT_H
