/*!
 * \file   ModbusUdpPort.h
 * \brief  Header file of class `ModbusUdpPort`.
 *
 * \author serhmarch
 * \date   August 2025
 */
#ifndef MODBUSUDPPORT_H
#define MODBUSUDPPORT_H

#include "ModbusUdpPortBase.h"

/*! \brief Class `ModbusUdpPort` implements UDP version of Modbus protocol.

    \details `ModbusUdpPort` provides client-side UDP communication for Modbus protocol over UDP/IP networks.
    It manages datagram-based communication with a remote Modbus endpoint, handling address resolution,
    data transmission, and protocol frame management including MBAP (Modbus Application Protocol) headers
    with automatic transaction ID management.

    The port can operate in blocking or non-blocking mode. In non-blocking mode, `open()` and I/O operations
    may return `Status_Processing` requiring repeated calls until completion. The class handles UDP socket
    lifecycle operations, including DNS resolution, socket creation, endpoint configuration, and cleanup.

    Transaction ID management:
    - Automatically increments transaction ID for each request (can be disabled)
    - Supports transaction ID repetition for retrying failed requests
    - Ensures proper request-response matching in concurrent environments

    Key features:
    - Configurable host (IP address or DNS name) and port number
    - Supports both blocking and non-blocking I/O modes
    - Automatic MBAP header construction and parsing
    - Built-in timeout handling for send and read operations
    - Buffer access methods for advanced protocol inspection
    - Connectionless datagram transport over UDP

 */

class MODBUS_EXPORT ModbusUdpPort : public ModbusUdpPortBase
{
public:
    /// \details Constructor of the class.
    ModbusUdpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::UDP`.
    Modbus::ProtocolType type() const override { return Modbus::UDP; }

public:
    ///  \details Repeat next request parameters (for Modbus UDP transaction Id).
    void setNextRequestRepeated(bool v);

    /// \details Returns `true' if the identifier of each subsequent parcel is automatically incremented by 1, `false' otherwise.
    bool autoIncrement() const;

    /// \details Returns the current transaction identifier.
    uint16_t transactionId() const;
    
    /// \details Sets the transaction identifier for the next request.
    void setTransactionId(uint16_t id);

protected:
    using ModbusUdpPortBase::ModbusUdpPortBase;
};

#endif // MODBUSUDPPORT_H
