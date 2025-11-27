/*!
 * \file   ModbusTcpPort.h
 * \brief  Header file of class `ModbusTcpPort`.
 *
 * \author serhmarch
 * \date   April 2024
 */
#ifndef MODBUSTCPPORT_H
#define MODBUSTCPPORT_H

#include "ModbusPort.h"

class ModbusTcpSocket;

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

class MODBUS_EXPORT ModbusTcpPort : public ModbusPort
{
public:
    /*! \brief `Defaults` class constain default settings values for `ModbusTcpPort`.
     */
    struct MODBUS_EXPORT Defaults
    {
        const Modbus::Char *host   ; ///< Default setting 'TCP host name (DNS or IP address)'
        const uint16_t      port   ; ///< Default setting 'TCP port number' for the listening server
        const uint32_t      timeout; ///< Default setting for the read timeout of every single conncetion

        ///  \details Constructor of the class.
        Defaults();

        /// \details Returns a reference to the global default value object.
        static const Defaults &instance();
    };

public:
    /// \details Constructor of the class.
    ModbusTcpPort(ModbusTcpSocket *socket, bool blocking = false);

    /// \details Constructor of the class.
    ModbusTcpPort(bool blocking = false);

    /// \details Destructor of the class. Close socket if it was not closed previously
    ~ModbusTcpPort();

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::TCP`.
    Modbus::ProtocolType type() const override { return Modbus::TCP; }

    /// \details Native OS handle for the socket.
    Modbus::Handle handle() const override;

    Modbus::StatusCode open() override;
    Modbus::StatusCode close() override;
    bool isOpen() const override;

public:
    ///  \details Returns the settings for the IP address or DNS name of the remote device.
    const Modbus::Char *host() const;

    ///  \details Sets the settings for the IP address or DNS name of the remote device.
    void setHost(const Modbus::Char *host);

    ///  \details Returns the setting for the TCP port number of the remote device.
    uint16_t port() const;

    ///  \details Sets the settings for the TCP port number of the remote device.
    void setPort(uint16_t port);

    ///  \details Repeat next request parameters (for Modbus TCP transaction Id).
    void setNextRequestRepeated(bool v) override;

    /// \details Returns `true' if the identifier of each subsequent parcel is automatically incremented by 1, `false' otherwise.
    bool autoIncrement() const;

    /// \details Returns the current transaction identifier.
    uint16_t transactionId() const;

public:
    const uint8_t *readBufferData() const override;
    uint16_t readBufferSize() const override;
    const uint8_t *writeBufferData() const override;
    uint16_t writeBufferSize() const override;

protected:
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) override;
    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override;

protected:
    using ModbusPort::ModbusPort;
};

#endif // MODBUSTCPPORT_H
