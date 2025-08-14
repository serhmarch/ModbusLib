/*!
 * \file   ModbusPort.h
 * \brief  Header file of abstract class `ModbusPort`.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSPORT_H
#define MODBUSPORT_H

#include <string>
#include <list>

#include "Modbus.h"

class ModbusPortPrivate;

/*! \brief The abstract class `ModbusPort` is the base class for a specific implementation of the Modbus communication protocol.

    \details `ModbusPort` contains general functions for working with a specific port, implementing a specific version of the Modbus communication protocol.
    For example, versions for working with a TCP port or a serial port.

 */
class MODBUS_EXPORT ModbusPort
{
public:
    /// \details Virtual destructor.
    virtual ~ModbusPort();

public:
    /// \details Returns the Modbus protocol type.
    virtual Modbus::ProtocolType type() const = 0;

    /// \details Returns the native handle value that depenp on OS used. For TCP it socket handle, for serial port - file handle.
    Modbus::Handle handle() const;

    /// \details Opens port (create connection) for further operations and returns the result status.
    Modbus::StatusCode open();

    /// \details Closes the port (breaks the connection) and returns the status the result status.
    Modbus::StatusCode close();

    /// \details Returns `true` if the port is open/communication with the remote device is established, `false` otherwise.
    bool isOpen() const;

    /// \details Implements the algorithm for writing to the port and returns the status of the operation.
    Modbus::StatusCode write();

    /// \details Implements the algorithm for reading from the port and returns the status of the operation.
    Modbus::StatusCode read();

    /// \details For the TCP version of the Modbus protocol. The identifier of each subsequent parcel is automatically increased by 1.
    /// If you set `setNextRequestRepeated(true)` then the next ID will not be increased by 1 but for only one next parcel.
    void setNextRequestRepeated(bool v);

public:
    /// \details Returns `true` if the port settings have been changed and the port needs to be reopened/reestablished communication with the remote device, `false` otherwise.
    bool isChanged() const;

    /// \details Returns `true` if the port works in server mode, `false` otherwise.
    bool isServerMode() const;

    /// \details Sets server mode if `true`, `false` for client mode.
    void setServerMode(bool mode);

    /// \details Returns `true` if the port works in synch (blocking) mode, `false` otherwise.
    bool isBlocking() const;

    /// \details Returns `true` if the port works in asynch (nonblocking) mode, `false` otherwise.
    bool isNonBlocking() const;

public: // settings
    ///  \details Returns the settings for the IP address or DNS name of the remote device.
    const Modbus::Char *host() const;

    ///  \details Sets the settings for the IP address or DNS name of the remote device.
    void setHost(const Modbus::Char *host);

    ///  \details Returns the setting for the TCP port number of the remote device.
    uint16_t port() const;

    ///  \details Sets the settings for the TCP port number of the remote device.
    void setPort(uint16_t port);

    ///  \details Returns the setting for the connection timeout of the remote device.
    uint32_t timeout() const;

    ///  \details Sets the setting for the connection timeout of the remote device.
    void setTimeout(uint32_t timeout);

    /// \details Returns current serial port name, e.g. `COM1` for Windows or `/dev/ttyS0` for Unix.
    const Modbus::Char *portName() const;

    /// \details Set current serial port name.
    void setPortName(const Modbus::Char *portName);

    /// \details Returns current serial port baud rate, e.g. 1200, 2400, 9600, 115200 etc.
    int32_t baudRate() const;

    /// \details Set current serial port baud rate.
    void setBaudRate(int32_t baudRate);

    /// \details Returns current serial port data bits, e.g. 5, 6, 7 or 8.
    int8_t dataBits() const;

    /// \details Set current serial port baud data bits.
    void setDataBits(int8_t dataBits);

    /// \details Returns current serial port `Modbus::Parity` enum value.
    Modbus::Parity parity() const;

    /// \details Set current serial port  `Modbus::Parity` enum value.
    void setParity(Modbus::Parity parity);

    /// \details Returns current serial port `Modbus::StopBits` enum value.
    Modbus::StopBits stopBits() const;

    /// \details Set current serial port `Modbus::StopBits` enum value.
    void setStopBits(Modbus::StopBits stopBits);

    /// \details Returns current serial port `Modbus::FlowControl` enum value.
    Modbus::FlowControl flowControl() const;

    /// \details Set current serial port `Modbus::FlowControl` enum value.
    void setFlowControl(Modbus::FlowControl flowControl);

    /// \details Returns current serial port timeout of waiting first byte of incomming packet (in milliseconds).
    inline uint32_t timeoutFirstByte() const { return timeout(); }

    /// \details Set current serial port timeout of waiting first byte of incomming packet (in milliseconds).
    inline void setTimeoutFirstByte(uint32_t timeout) { setTimeout(timeout); }

    /// \details Returns current serial port timeout of waiting next byte (inter byte waiting tgimeout) of incomming packet (in milliseconds).
    uint32_t timeoutInterByte() const;

    /// \details Set current serial port timeout of waiting next byte (inter byte waiting tgimeout) of incomming packet (in milliseconds).
    void setTimeoutInterByte(uint32_t timeout);

public: // errors
    /// \details Returns the status of the last error of the performed operation.
    Modbus::StatusCode lastErrorStatus() const;

    /// \details Returns the pointer to `const Char` text buffer of the last error of the performed operation.
    const Modbus::Char *lastErrorText() const;

public:
    /// \details The function places a raw packet in the buffer for further sending. Returns the status of the operation.
    Modbus::StatusCode writeRawBuffer(const void *buff, uint16_t szInBuff);

    /// \details The function copies input packet that the `read()` function puts into the inner buffer to the output `buff` 
    /// and set it size into the output varaible `szOutBuff`. Returns the status of the operation.
    Modbus::StatusCode readRawBuffer(void *buff, uint16_t maxSzBuff, uint16_t *szOutBuff);
    
    /// \details The function directly generates a packet and places it in the buffer for further sending. Returns the status of the operation.
    virtual Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) = 0;

    /// \details The function parses the packet that the `read()` function puts into the buffer, checks it for correctness, extracts its parameters, and returns the status of the operation.
    virtual Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) = 0;
    
public: // buffer
    /// \details Returns pointer to data of read buffer.
    const uint8_t *readBufferData() const;

    /// \details Returns maximum size of read buffer.
    uint16_t readBufferMaxSize() const;

    /// \details Returns current size of read buffer.
    uint16_t readBufferSize() const;

    /// \details Returns pointer to data of write buffer.
    const uint8_t *writeBufferData() const;

    /// \details Returns maximum size of write buffer.
    uint16_t writeBufferMaxSize() const;

    /// \details Returns size of data of write buffer.
    uint16_t writeBufferSize() const;

protected:
    /// \details Sets the error parameters of the last operation performed.
    Modbus::StatusCode setError(Modbus::StatusCode status, const Modbus::Char *text);

protected:
    /// \cond
    ModbusPortPrivate *d_ptr;
    ModbusPort(ModbusPortPrivate *d);
    /// \endcond
};

#endif // MODBUSPORT_H
