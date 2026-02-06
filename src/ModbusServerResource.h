/*!
 * \file   ModbusServerResource.h
 * \brief  The header file defines the class that controls specific port
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSSERVERRESOURCE_H
#define MODBUSSERVERRESOURCE_H

#include "ModbusServerPort.h"

class ModbusPort;

/*! \brief Implements direct control for `ModbusPort` derived classes (TCP or serial) for server side.

    \details `ModbusServerResource` derived from `ModbusServerPort` and
    makes `ModbusPort` object behaves like server port.
    Pointer to `ModbusPort` object is passed to `ModbusServerResource` constructor.
    `ModbusServerResource` owns the passed `ModbusPort` object and deletes it in destructor.

    Also `ModbusServerResource` have `ModbusInterface` object as second parameter
    of constructor which process every Modbus function request.
    `ModbusServerResource` does not own the passed `ModbusInterface` object.

    Key characteristics:
    - Concrete implementation of ModbusServerPort for single-connection scenarios
    - Wraps any ModbusPort (TCP, RTU, ASCII) for server-side operation
    - Delegates all Modbus function processing to ModbusInterface device
    - Manages complete request-response lifecycle
    - Supports all standard Modbus functions through device delegation
    - Works with any protocol through polymorphic port interface
    - Ideal for simple server implementations and serial protocols
    
    This implementation provides:
    - Automatic server mode configuration for the wrapped port
    - Complete request processing pipeline (input → device → output)
    - Protocol-agnostic operation through ModbusPort abstraction
    - Pass-through of port configuration (timeout, type, open/close)
    - Three-stage processing: input parsing, device execution, output assembly
    - Error handling and status propagation throughout processing stages
    - Signal forwarding from underlying port (open/close, Tx/Rx, errors)
    
    Request processing workflow:
    1. process() is called repeatedly in application loop
    2. Port reads incoming data (processInputData)
    3. Request is parsed and validated
    4. Device processes the Modbus function (processDevice)
    5. Response is assembled (processOutputData)
    6. Port writes response back to client
    
    The three-stage processing architecture:
    - processInputData(): Receives and parses incoming request from port buffer
    - processDevice(): Delegates function execution to ModbusInterface device
    - processOutputData(): Assembles and prepares response in port buffer
    
    This separation allows derived classes to customize each stage while maintaining
    the overall processing flow.
    
    Usage scenarios:
    - Serial port servers (RTU/ASCII) with single connection
    - Simple TCP servers for single-client applications
    - Testing and development environments
    - Embedded systems with resource constraints
    - Protocol adapters and gateways
    
    For TCP servers requiring multiple simultaneous connections, use ModbusTcpServer
    instead, which manages multiple connections with separate ModbusServerResource
    instances per connection.

 */
class MODBUS_EXPORT ModbusServerResource : public ModbusServerPort
{
public:
    /// \details Constructor of the class.
    /// \param[in]  port    Pointer to the `ModbusPort` which is managed by the current class object.
    /// `ModbusServerResource` owns the passed `ModbusPort` object and deletes it in destructor.
    /// \param[in]  device  Pointer to the `ModbusInterface` implementation to which all requests for Modbus functions are forwarded.
    /// `ModbusServerResource` does not own the passed `ModbusInterface` object.
    ModbusServerResource(ModbusPort *port, ModbusInterface *device);

public:
    /// \details Returns pointer to inner port which was previously passed in constructor.
    ModbusPort *port() const;

public: // server port interface
    /// \details Returns type of Modbus protocol. Same as `port()->type()`.
    Modbus::ProtocolType type() const override;

    Modbus::StatusCode open() override;

    Modbus::StatusCode close() override;

    bool isOpen() const override;

    ///  \details Returns the timeout for the inner port object defined in constructor.
    uint32_t timeout() const override;

    ///  \details Sets the timeout for the inner port object defined in constructor.
    void setTimeout(uint32_t timeout) override;

    Modbus::StatusCode process() override;

    const Modbus::Char *lastErrorText() const override;

protected:
    /// \details Process input data `buff` with `size` and returns status of the operation.
    virtual Modbus::StatusCode processInputData(const uint8_t *buff, uint16_t sz);

    /// \details Transfer input request Modbus function to inner device and returns status of the operation.
    virtual Modbus::StatusCode processDevice();

    /// \details Process output data `buff` with `size` and returns status of the operation.
    virtual Modbus::StatusCode processOutputData(uint8_t *buff, uint16_t &sz);

protected:
    using ModbusServerPort::ModbusServerPort;
};

#endif // MODBUSSERVERRESOURCE_H
