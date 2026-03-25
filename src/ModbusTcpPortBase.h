/*!
 * \file   ModbusTcpPortBase.h
 * \brief  Header file of class `ModbusTcpPortBase`.
 *
 * \author serhmarch
 * \date   Feb 2026
 */
#ifndef MODBUSTCPPORTBASE_H
#define MODBUSTCPPORTBASE_H

#include "ModbusNetPort.h"

class ModbusSocket;

/*! \brief Class `ModbusTcpPortBase` implements TCP transport of Modbus protocol.

    \details `ModbusTcpPortBase` derived from `ModbusNetPort` and provides a common TCP
    transport layer for Modbus protocol implementations. It encapsulates socket-based networking
    logic and exposes a unified interface for opening and closing a TCP connection, checking
    connection state, and performing frame transmission and reception.

    This class is intended as a reusable base for protocol-specific TCP ports (for example,
    ASC over TCP and RTU over TCP). Derived classes are responsible for protocol framing and
    payload interpretation, while `ModbusTcpPortBase` manages transport-level I/O behavior.

    Both blocking and non-blocking socket modes are supported through inherited construction
    parameters, allowing applications to choose the most suitable I/O strategy.
 */

class MODBUS_EXPORT ModbusTcpPortBase : public ModbusNetPort
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

#endif // MODBUSTCPPORTBASE_H
