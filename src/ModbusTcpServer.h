/*!
 * \file   ModbusTcpServer.h
 * \brief  Header file of Modbus TCP server.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSSERVERTCP_H
#define MODBUSSERVERTCP_H

#include "ModbusServerPort.h"

class ModbusTcpSocket;

/*! \brief The `ModbusTcpServer` class implements TCP server part of the Modbus protocol.

    \details `ModbusTcpServer` manages multiple TCP connections and processes Modbus requests from clients.
    It listens on a specified TCP port, accepts incoming connections, and creates `ModbusServerResource` 
    instances for each connection to handle Modbus protocol communication. The server manages connection 
    lifecycle, enforces maximum connection limits, and propagates settings (timeout, broadcast, unit map) 
    to all active connections. Incoming requests are forwarded to the `ModbusInterface` device object 
    provided in the constructor.
    
    The server operates asynchronously through the `process()` method which must be called repeatedly 
    in a loop. It handles the complete server lifecycle including socket creation, binding, listening, 
    accepting new connections, processing existing connections, and cleanup. Each connection runs 
    independently with its own `ModbusServerResource` that processes Modbus protocol frames.
    
    Key features:
    - Automatic connection management with configurable maximum connections limit
    - Non-blocking operation suitable for single-threaded event loops
    - Virtual methods `createTcpPort()` and `deleteTcpPort()` allow customization of connection handling
    - Signals for connection events: `signalNewConnection()`, `signalCloseConnection()`
    - Inherits standard server signals from base class: `signalOpened()`, `signalClosed()`, `signalError()`, `signalTx()`, `signalRx()`
    - Supports broadcast mode and unit address filtering through unit map
    - Thread-safe for single-threaded usage (caller responsible for thread synchronization if needed)
    
    Example usage:
    \code
    // Define device that implements ModbusInterface
    class MyDevice : public ModbusInterface
    {
    public:
        StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override
        {
            // Read data from your device memory/registers
            for (uint16_t i = 0; i < count; i++)
                values[i] = myRegisters[offset + i];
            return Status_Good;
        }
        // Implement other ModbusInterface methods...
    private:
        uint16_t myRegisters[1000];
    };
    
    // Create and configure TCP server
    MyDevice device;
    ModbusTcpServer server(&device);
    server.setPort(502);           // Standard Modbus TCP port
    server.setTimeout(5000);       // 5 second timeout
    server.setMaxConnections(10);  // Allow up to 10 simultaneous connections
    
    // Open server
    StatusCode status = server.open();
    while (StatusIsProcessing(status))
        status = server.process();
    
    if (StatusIsGood(status))
    {
        // Main server loop
        while (running)
        {
            server.process();  // Handle all connections
            // Add your application logic here
        }
        
        // Clean shutdown
        server.close();
    }
    \endcode

 */

class MODBUS_EXPORT ModbusTcpServer : public ModbusServerPort
{
public:
    /*! \brief `Defaults` class constain default settings values for `ModbusTcpServer`.
     */
    struct MODBUS_EXPORT Defaults
    {
        const Modbus::Char *ipaddr ; ///< Default setting 'IP address' to bind the server
        const uint16_t      port   ; ///< Default setting 'TCP port number' for the listening server
        const uint32_t      timeout; ///< Default setting for the read timeout of every single conncetion
        const uint32_t      maxconn; ///< Default setting for the maximum number of simultaneous connections to the server

        ///  \details Constructor of the class.
        Defaults();

        /// \details Returns a reference to the global default value object.
        static const Defaults &instance();
    };

public:
    ///  \details Constructor of the class. `device` param is object which might process incoming requests for read/write memory.
    ModbusTcpServer(ModbusInterface *device);

    ///  \details Destructor of the class. Clear all unclosed connections.
    ~ModbusTcpServer();

public:
    ///  \details Returns the settings for the IP address to bind the server.
    const Modbus::Char *ipaddr() const;

    ///  \details Sets the settings for the IP address to bind the server.
    void setIpaddr(const Modbus::Char *ipaddr);

    ///  \details Returns the setting for the TCP port number of the server.
    uint16_t port() const;

    ///  \details Sets the settings for the TCP port number of the server.
    void setPort(uint16_t port);

    ///  \details Returns the setting for the read timeout of every single conncetion.
    uint32_t timeout() const override;

    ///  \details Sets the setting for the read timeout of every single conncetion.
    void setTimeout(uint32_t timeout) override;

    ///  \details Returns setting for the maximum number of simultaneous connections to the server.
    uint32_t maxConnections() const;

    ///  \details Sets the setting for the maximum number of simultaneous connections to the server.
    void setMaxConnections(uint32_t maxconn);

public:
    /// \details Returns the Modbus protocol type. In this case it is `Modbus::TCP`.
    Modbus::ProtocolType type() const override { return Modbus::TCP; }

    /// \details Returns `true`.
    bool isTcpServer() const override { return true; }

    /// \details Try to listen for incoming connections on TCP port that was previously set (`port()`).
    /// \returns \li `Modbus::Status_Good` on success
    ///          \li `Modbus::Status_Processing` when operation is not complete
    ///          \li `Modbus::Status_BadTcpCreate` when can't create TCP socket
    ///          \li `Modbus::Status_BadTcpBind` when can't bind TCP socket
    ///          \li `Modbus::Status_BadTcpListen` when can't listen TCP socket
    Modbus::StatusCode open() override;

    /// \details Stop listening for incoming connections and close all previously opened connections.
    /// \returns \li `Modbus::Status_Good` on success
    ///          \li `Modbus::Status_Processing` when operation is not complete
    Modbus::StatusCode close() override;

    /// \details Returns `true` if the server is currently listening for incoming connections, `false` otherwise.
    bool isOpen() const override;

    void setBroadcastEnabled(bool enable) override;

    void setUnitMap(const void *unitmap) override;

    void setUnitEnabled(uint8_t unit, bool enable) override;

    /// \details Main function of TCP server. Must be called in cycle to perform all incoming TCP connections.
    Modbus::StatusCode process() override;
    
public:
    /// \details Creates `ModbusServerPort` for new incoming connection defined by `ModbusTcpSocket` pointer
    /// May be reimplemented in subclasses.
    virtual ModbusServerPort *createTcpPort(ModbusTcpSocket *socket);
    
    /// \details Deletes `ModbusServerPort` by default. 
    /// May be reimplemented in subclasses.
    virtual void deleteTcpPort(ModbusServerPort *port);
    
public: // SIGNALS
    /// \details Signal occured when new TCP connection was accepted. `source` - name of the current connection.
    void signalNewConnection(const Modbus::Char *source);

    /// \details Signal occured when TCP connection was closed. `source` - name of the current connection.
    void signalCloseConnection(const Modbus::Char *source);

protected:
    /// \details Checks for incoming connections and returns pointer `ModbusTcpSocket` if new connection established, `nullptr` otherwise.
    ModbusTcpSocket *nextPendingConnection();

    /// \details Clear all allocated memory for previously established connections.
    void clearConnections();

protected:
    using ModbusServerPort::ModbusServerPort;
};

#endif // MODBUSSERVERTCP_H
