/*!
 * \file   ModbusRtuOverUdpPort.h
 * \brief  Contains definition of RTU over UDP port class.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSRTUOVERUDPPORT_H
#define MODBUSRTUOVERUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Implements RTU over UDP version of the Modbus communication protocol.

    \details `ModbusRtuOverUdpPort` derived from `ModbusUdpPortBase` and implements RTU framing
    for Modbus communication over UDP. This protocol combines the compact binary encoding of
    Modbus RTU with the network transport capabilities of UDP, providing efficient data transfer
    while maintaining network connectivity.

    The RTU variant of Modbus encodes protocol data in binary form and includes CRC-based integrity
    checking, making it suitable for bandwidth-efficient communication and interoperability with
    devices that rely on RTU framing. When combined with UDP, this class enables RTU-framed Modbus
    protocol transmission over UDP/IP networks, facilitating integration with systems that require
    RTU-compatible payloads across network infrastructure.

    This implementation supports connectionless communication over UDP datagrams. The class
    handles frame boundaries and validation for RTU-formatted messages, ensuring proper message
    framing and integrity across UDP packets. Both blocking and non-blocking socket modes are
    supported to accommodate different application requirements and I/O patterns.

 */
class MODBUS_EXPORT ModbusRtuOverUdpPort : public ModbusUdpPortBase
{
public:
    ///  \details Constructor of the class. if `blocking = true` then defines blocking mode, non blocking otherwise.
    ModbusRtuOverUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. For `ModbusRtuOverUdpPort` returns `Modbus::RTUvUDP`.
    Modbus::ProtocolType type() const override { return Modbus::RTUvUDP; }

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSRTUOVERUDPPORT_H
