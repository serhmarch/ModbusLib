/*!
 * \file   ModbusAscPort.h
 * \brief  Contains definition of base server side port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSSERVERPORT_H
#define MODBUSSERVERPORT_H

#include <cstdlib>  // for malloc, free
#include <cstring>  // for memcpy

#include "ModbusObject.h"

/*! \brief Abstract base class for direct control of `ModbusPort` derived classes (TCP or serial) for server side.

    \details Pointer to `ModbusPort` object must be passed to `ModbusServerPort`
    derived class constructor.

    Also assumed that `ModbusServerPort` derived classes must accept `ModbusInterface`
    object in its constructor to process every Modbus function request.

    Key characteristics:
    - Abstract base class for server-side port implementations
    - Controls underlying ModbusPort for server operation
    - Delegates Modbus function processing to ModbusInterface device
    - Supports unit address filtering through unit map mechanism
    - Provides broadcast mode support for unit address 0
    - Event-driven architecture with signal callbacks
    - Asynchronous operation through process() method
    
    This base class provides:
    - Device management for request delegation to ModbusInterface implementation
    - Port lifecycle control (open, close, state checking)
    - Timeout configuration for read operations
    - Unit map filtering (256-bit bitmap for addresses 0-255)
    - Broadcast mode enablement for address 0
    - Context storage for user-defined data association
    - Signal emission for open/close, Tx/Rx, and error events
    - State management for connection status
    
    The class establishes the server-side architecture:
    1. ModbusServerPort wraps a ModbusPort and controls it in server mode
    2. Incoming requests are read from the port
    3. Requests are delegated to the ModbusInterface device for processing
    4. Responses are written back through the port
    5. Signals notify about communication events
    
    Unit map filtering:
    The unit map is a 32-byte bitmap (256 bits) where each bit represents a unit address.
    - Bit set (1): Unit address is enabled and requests will be processed
    - Bit clear (0): Unit address is disabled and requests will be ignored
    - Not set (nullptr): All unit addresses are accepted (default behavior)
    
    This allows selective enabling of unit addresses, useful when a single server
    manages multiple logical devices or when security requires limiting accessible units.
    
    Process-driven operation:
    The process() method must be called repeatedly in the application's main loop.
    It performs non-blocking I/O operations and request handling, returning status
    codes that indicate the current state (good, processing, error). This design
    enables integration with event loops and asynchronous architectures without
    blocking the application.
    
    Context storage:
    The context pointer allows applications to associate custom data with the server
    port instance, useful for callbacks and signal handlers that need access to
    application-specific state without global variables.

 */
class MODBUS_EXPORT ModbusServerPort : public ModbusObject
{
public:
    /// \details Returns pointer to `ModbusInterface` object/device that was previously passed in constructor.
    /// This device must process every input Modbus function request for this server port.
    ModbusInterface *device() const;

    /// \details Set pointer to `ModbusInterface` object/device to transfer all request ot it.
    /// This device must process every input Modbus function request for this server port.
    void setDevice(ModbusInterface *device);

public: // server port interface
    /// \details Returns type of Modbus protocol.
    virtual Modbus::ProtocolType type() const = 0;

    /// \details Returns `true` if current server port is TCP server, `false` otherwise.
    virtual bool isTcpServer() const;

    /// \details Open inner port/connection to begin working and returns status of the operation. 
    /// User do not need to call this method directly.
    virtual Modbus::StatusCode open() = 0;

    /// \details Closes port/connection and returns status of the operation.
    virtual Modbus::StatusCode close() = 0;

    /// \details Returns `true` if inner port is open, `false` otherwise.
    virtual bool isOpen() const = 0;

    ///  \details Returns the setting for the timeout of the port. 
    virtual uint32_t timeout() const = 0;

    ///  \details Sets the setting for the read timeout of every single conncetion.
    virtual void setTimeout(uint32_t timeout) = 0;

    /// \details Returns `true` if broadcast mode for `0` unit address is enabled, `false` otherwise.
    /// Broadcast mode for `0` unit address is required by Modbus protocol so it is enabled by default
    bool isBroadcastEnabled() const;

    /// \details Enables broadcast mode for `0` unit address. It is enabled by default.
    /// \sa `isBroadcastEnabled()`
    virtual void setBroadcastEnabled(bool enable);

    /// \details Return pointer to the units map byte array of the current server. 
    /// By default unit map is not set so return value is `nullptr`.
    /// Unit map is data type with size of 32 bytes in which every bit represents unit address from `0` to `255`.
    /// So bit 0 of byte 0 represents unit address `0`, bit 1 of byte 0 represents unit address `1` and so on.
    /// Bit 0 of byte 1 represnt unit address `8`, bit 7 of byte 31 represents unit address `255`.
    /// If set unit map can enable or disable (depends on respecting 1/0 bit value) unit address for further processing.
    /// It is not set by default and function returns `nullptr`.
    const void *unitMap() const;

    /// \details Set units map of current server. Server make a copy of units map data.
    /// Unit map is data type with size of at least 32 bytes (MB_UNITMAP_SIZE)
    /// in which every bit represents unit address from `0` to `255`.
    /// If `unitmap` is `nullptr` then previous unit map will be deleted and
    /// all unit addresses are enabled for processing.
    /// \sa `unitMap()`
    virtual void setUnitMap(const void *unitmap);

    /// \details Clear units map of current server. All unit addresses will be enabled for processing.
    /// Equivalent to `setUnitMap(nullptr)`.
    inline void clearUnitMap() { setUnitMap(nullptr); }

    /// \details Return unit map bit array string repr like "1,3-8,10"
    Modbus::String unitMapString() const;

    /// \details Set units map of current server as string like "1,3-8,10"
    void setUnitMapString(const Modbus::Char *s);

    /// \details Set units map of current server as string like "1,3-8,10"
    inline void setUnitMapString(const Modbus::String &s) {  setUnitMapString(s.data()); }

    /// \details Returns `true` if unit address `unit` is enabled for processing, `false` otherwise.
    /// If unit map is not set (nullptr) then all unit addresses are enabled by default.
    /// If broadcast mode is enabled then function always returns `true` for unit address `0`.
    bool isUnitEnabled(uint8_t unit) const;

    /// \details Enable or disable unit address `unit` for processing.
    /// If unit map was not set previously it will be created automatically and
    /// all unit addresses will be disabled by default except the `unit` address that is enabled/disabled by this function.
    virtual void setUnitEnabled(uint8_t unit, bool enable);

    /// \details Return context of the port previously set by `setContext` function or `nullptr` by default.
    void *context() const;

    /// \details Set context of the port. 
    void setContext(void *context);

    /// \details Main function of the class. Must be called in the cycle. 
    /// Return statuc code is not very useful but can indicate that inner server operations are good, bad or in process.
    virtual Modbus::StatusCode process() = 0;

public:
    /// \details Returns the status of the last operation performed.
    Modbus::StatusCode lastStatus() const;

    /// \details Returns the timestamp of the last operation performed.
    Modbus::Timestamp lastStatusTimestamp() const;

    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

    /// \details Returns the text of the last error of the performed operation.
    virtual const Modbus::Char *lastErrorText() const;

public:
    /// \details Returns `true` if current port has closed inner state, `false` otherwise.
    bool isStateClosed() const;

public: // SIGNALS
    /// \details Signal occured when inner port was opened. `source` - current port name.
    void signalOpened(const Modbus::Char *source);

    /// \details Signal occured when inner port was closed. `source` - current port name.
    void signalClosed(const Modbus::Char *source);

    /// \details Signal occured when  the original packet 'Tx' from the internal list of callbacks, 
    /// passing them the original array 'buff' and its size 'size'. `source` - current port name.
    void signalTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Signal occured when  the incoming packet 'Rx' from the internal list of callbacks, 
    /// passing them the input array 'buff' and its size 'size'. `source` - current port name.
    void signalRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

    /// \details Signal occured when  error is occured with error's `status` and `text`. `source` - current port name.
    void signalError(const Modbus::Char *source, Modbus::StatusCode status, const Modbus::Char *text);

    /// \details Calls each callback of the port when operation is completed.
    void signalCompleted(const Modbus::Char *source, Modbus::StatusCode status);

protected:
    using ModbusObject::ModbusObject;
};

#endif // MODBUSSERVERPORT_H

