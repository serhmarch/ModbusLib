/*!
 * \file   ModbusSerialPort.h
 * \brief  Contains definition of base serial port class.
 *
 * \author serhmarch
 * \date   May 2024
 */
#ifndef MODBUSSERIALPORT_H
#define MODBUSSERIALPORT_H

#include "ModbusPort.h"

/*! \brief The abstract class `ModbusSerialPort` is the base class serial port Modbus communications.

    \details The abstract class `ModbusSerialPort` is the base class for a specific implementation of 
    the Modbus communication protocol that using Serial Port. It implements functions which are common
    for the serial port: `open`, `close`, `read` and `write`.

    Key characteristics:
    - Abstract base class for serial communication protocols (RTU and ASCII)
    - Implements common serial port operations (open, close, read, write)
    - Provides comprehensive serial port configuration (baud rate, data bits, parity, stop bits, flow control)
    - Supports configurable timeout mechanisms for byte-level reception
    - Platform-independent interface with OS-specific implementation
    - Manages native serial port handles transparently
    
    This base class provides:
    - Complete serial port lifecycle management (open, close, state checking)
    - Configuration for all standard serial parameters (9600 8N1, etc.)
    - Dual timeout mechanism: first byte timeout and inter-byte timeout
    - Support for various baud rates (1200 to 115200 and beyond)
    - Support for various data formats (5-8 data bits, parity options, stop bits)
    - Flow control options (None, Hardware, Software)
    - Buffer access methods for protocol implementation
    - Native handle access for platform-specific operations
    
    Derived classes implement protocol-specific features:
    - ModbusRtuPort: Binary RTU protocol with CRC-16 and 3.5 character timing
    - ModbusAscPort: ASCII protocol with hexadecimal encoding and LRC
    
    Serial port configuration pattern:
    The class provides granular control over all serial parameters, allowing
    precise configuration for different devices and communication requirements.
    Common configurations include:
    - RTU: 9600 baud, 8 data bits, No parity, 1 stop bit (9600 8N1)
    - ASCII: 9600 baud, 7 data bits, Even parity, 1 stop bit (9600 7E1)
    
    Timeout mechanism:
    Two distinct timeout values control reception behavior:
    1. timeoutFirstByte: Maximum time to wait for the first byte of a response
       (typically longer to account for server processing time)
    2. timeoutInterByte: Maximum time between consecutive bytes within a packet
       (typically shorter to detect transmission errors quickly)
    
    This dual-timeout approach ensures robust communication while minimizing
    latency in error detection and recovery.

 */
class MODBUS_EXPORT ModbusSerialPort : public ModbusPort
{
public:
/*! \brief Holds the default values of the settings.
*/
    struct MODBUS_EXPORT Defaults
    {
        const Modbus::Char       *portName        ; ///< Default value for the serial port name 
        const int32_t             baudRate        ; ///< Default value for the serial port's baud rate
        const int8_t              dataBits        ; ///< Default value for the serial port's data bits
        const Modbus::Parity      parity          ; ///< Default value for the serial port's patiry
        const Modbus::StopBits    stopBits        ; ///< Default value for the serial port's stop bits
        const Modbus::FlowControl flowControl     ; ///< Default value for the serial port's flow control
        const uint32_t            timeoutFirstByte; ///< Default value for the serial port's timeout waiting first byte of packet
        const uint32_t            timeoutInterByte; ///< Default value for the serial port's timeout waiting next byte of packet

        /// \details Constructor ot the class.
        Defaults();
        
        /// \details Returns a reference to the global `ModbusSerialPort::Defaults` object.
        static const Defaults &instance();
    };

public:
    /// \details Virtual destructor. Closes serial port before destruction.
    ~ModbusSerialPort();

public:
    /// \details Returns native OS serial port handle, e.g. `HANDLE` value for Windows.
    Modbus::Handle handle() const override;

    /// \details Try to open serial port and returns `Modbus::Status_Good` if success or `Modbus::Status_BadSerialOpen` otherwise.
    Modbus::StatusCode open() override;

    /// \details Close serial port and returns `Modbus::Status_Good`.
    Modbus::StatusCode close() override;

    /// \details Returns `true` if the serial port is open, `false` otherwise.
    bool isOpen() const override;

public: // settings
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

public:
    const uint8_t *readBufferData() const override;
    uint16_t readBufferSize() const override;
    const uint8_t *writeBufferData() const override;
    uint16_t writeBufferSize() const override;

protected:
    Modbus::StatusCode write() override;
    Modbus::StatusCode read() override;

protected:
    /// \cond
    using ModbusPort::ModbusPort;
    /// \endcond
};

#endif // MODBUSSERIALPORT_H
