/*!
 * \file   ModbusTcpPort.h
 * \brief  Header file of class `ModbusTcpPort`.
 *
 * \author serhmarch
 * \date   April 2024
 */
#ifndef MODBUSTCPPORT_H
#define MODBUSTCPPORT_H

#include "ModbusTcpPortBase.h"

class ModbusSocket;

/*! \brief Class `ModbusTcpPort` implements TCP version of Modbus protocol.

    \details `ModbusTcpPort` provides client-side TCP communication for Modbus protocol over TCP/IP networks.
    It manages a single TCP socket connection to a remote Modbus server, handling connection establishment,
    data transmission, and protocol frame management including MBAP (Modbus Application Protocol) headers
    with automatic transaction ID management.
    
    The port can operate in blocking or non-blocking mode. In non-blocking mode, `open()` and I/O operations
    may return `Status_Processing` requiring repeated calls until completion. The class automatically handles
    TCP connection lifecycle, including DNS resolution, socket creation, connection establishment, and cleanup.
    
    Transaction ID management:
    - Automatically increments transaction ID for each request (can be disabled)
    - Supports transaction ID repetition for retrying failed requests
    - Ensures proper request-response matching in concurrent environments
    
    Key features:
    - Configurable host (IP address or DNS name) and port number
    - Supports both blocking and non-blocking I/O modes
    - Automatic MBAP header construction and parsing
    - Built-in timeout handling for connection and read operations
    - Buffer access methods for advanced protocol inspection
    - Can be constructed with existing socket for server-side usage
 */

class MODBUS_EXPORT ModbusTcpPort : public ModbusTcpPortBase
{
public:
    /// \details Constructor of the class.
    ModbusTcpPort(ModbusSocket *socket, bool blocking = false);

    /// \details Constructor of the class.
    ModbusTcpPort(bool blocking = false);

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::TCP`.
    Modbus::ProtocolType type() const override { return Modbus::TCP; }

public:
    ///  \details Repeat next request parameters (for Modbus TCP transaction Id).
    void setNextRequestRepeated(bool v);

    /// \details Returns `true' if the identifier of each subsequent parcel is automatically incremented by 1, `false' otherwise.
    bool autoIncrement() const;

    /// \details Returns the current transaction identifier.
    uint16_t transactionId() const;
    
    /// \details Sets the transaction identifier for the next request.
    void setTransactionId(uint16_t id);

protected:
    using ModbusTcpPortBase::ModbusTcpPortBase;
};

#endif // MODBUSTCPPORT_H
