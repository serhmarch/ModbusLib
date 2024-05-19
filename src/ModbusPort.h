/*!
 * \file   ModbusPort.h
 * \brief  Header file of abstract class `ModbusPort`.
 *
 * \author march
 * \date   April 2024
 */
#ifndef MODBUSPORT_H
#define MODBUSPORT_H

#include <string>
#include <list>

#include "ModbusObject.h"

/*! \brief The abstract class `ModbusPort` is the base class for a specific implementation of the Modbus communication protocol.

    \details `ModbusPort` contains general functions for working with a specific port, implementing a specific version of the Modbus communication protocol.
    For example, versions for working with a TCP port or a serial port.

 */

class ModbusPortPrivate;

class MODBUS_EXPORT ModbusPort
{
public:
    virtual ~ModbusPort();

public:
    /// \details Returns the Modbus protocol type.
    virtual Modbus::ProtocolType type() const = 0;

    /// \details Returns the native handle value that depenp on OS used. For TCP it socket handle, for serial port - file handle.
    virtual Modbus::Handle handle() const = 0;

    /// \details Opens port (create connection) for further operations and returns the result status.
    virtual Modbus::StatusCode open() = 0;

    /// \details Closes the port (breaks the connection) and returns the status the result status.
    virtual Modbus::StatusCode close() = 0;

    /// \details Returns `true` if the port is open/communication with the remote device is established, `false` otherwise.
    virtual bool isOpen() const = 0;

    /// \details For the TCP version of the Modbus protocol. The identifier of each subsequent parcel is automatically increased by 1.
    /// If you set `setNextRequestRepeated(true)` then the next ID will not be increased by 1 but for only one next parcel.
    virtual void setNextRequestRepeated(bool v);

public:
    /// \details Returns `true` if the port settings have been changed and the port needs to be reopened/reestablished communication with the remote device, `false` otherwise.
    bool isChanged() const;

    /// \details Returns `true` if the port works in server mode, `false` otherwise.
    bool isServerMode() const;

    /// \details Sets server mode if `true`, `false` for client mode.
    virtual void setServerMode(bool mode);

    /// \details Returns `true` if the port works in synch (blocking) mode, `false` otherwise.
    bool isBlocking() const;

    /// \details Returns `true` if the port works in asynch (nonblocking) mode, `false` otherwise.
    bool isNonBlocking() const;

public: // errors
    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

    /// \details Returns the pointer to `const Char` text buffer of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

public:
    /// \details Returns `true` if the port is blocked - it has already received a request and is processing it, `false` otherwise.
    bool isWriteBufferBlocked() const;

    /// \details Unblocks the port and makes it ready to receive the next request.
    void freeWriteBuffer();

    /// \details The function directly generates a packet and places it in the buffer for further sending. Returns the status of the operation.
    virtual Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) = 0;

    /// \details The function parses the packet that the `read()` function puts into the buffer, checks it for correctness, extracts its parameters, and returns the status of the operation.
    virtual Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) = 0;
    
    /// \details Implements the algorithm for writing to the port and returns the status of the operation.
    virtual Modbus::StatusCode write() = 0;

    /// \details Implements the algorithm for reading from the port and returns the status of the operation.
    virtual Modbus::StatusCode read() = 0;

public: // buffer
    virtual const uint8_t *readBufferData() = 0;
    virtual uint16_t readBufferSize() = 0;
    virtual const uint8_t *writeBufferData() = 0;
    virtual uint16_t writeBufferSize() = 0;

protected:
    /// \details Sets the error parameters of the last operation performed.
    Modbus::StatusCode setError(Modbus::StatusCode status, const Modbus::Char *text);

    /// \details If `setChanged(true)` - sets the sign that the port settings have been changed and it is necessary to reopen the port/reestablish communication with the remote device.
    /// If `setChanged(false)` - clears this sign.
    void setChanged(bool changed = true);

protected:
    ModbusPortPrivate *d_ptr;
    ModbusPort(ModbusPortPrivate *d);
};

#endif // MODBUSPORT_H
