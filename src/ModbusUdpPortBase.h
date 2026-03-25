/*!
 * \file   ModbusUdpPortBase.h
 * \brief  Header file of class `ModbusUdpPortBase`.
 *
 * \author serhmarch
 * \date   Feb 2026
 */
#ifndef MODBUSUDPPORTBASE_H
#define MODBUSUDPPORTBASE_H

#include "ModbusNetPort.h"

class ModbusSocket;

/*! \brief Class `ModbusUdpPortBase` implements UDP transport of Modbus protocol.

    \details `ModbusUdpPortBase` derived from `ModbusNetPort` and provides a common UDP
    transport layer for Modbus protocol implementations. It encapsulates socket-based networking
    logic and exposes a unified interface for opening and closing UDP communication, checking
    communication state, and performing frame transmission and reception.

    This class is intended as a reusable base for protocol-specific UDP ports (for example,
    ASC over UDP and RTU over UDP). Derived classes are responsible for protocol framing and
    payload interpretation, while `ModbusUdpPortBase` manages transport-level I/O behavior.

    UDP communication is connectionless and datagram-oriented, so message delivery and ordering
    are governed by network conditions and higher-level protocol handling. Both blocking and
    non-blocking socket modes are supported through inherited construction parameters, allowing
    applications to choose the most suitable I/O strategy.
 */

class MODBUS_EXPORT ModbusUdpPortBase : public ModbusNetPort
{
public:
    Modbus::Handle handle() const override;
    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

protected:
    using ModbusNetPort::ModbusNetPort;
};

#endif // MODBUSUDPPORTBASE_H
