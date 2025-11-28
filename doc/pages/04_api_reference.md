# API Reference {#api_reference}

[TOC]

This document provides a comprehensive reference for the ModbusLib C++ API. ModbusLib is a cross-platform library implementing the Modbus communication protocol with support for TCP, RTU, and ASCII variants.

---

## Module Organization {#api-module-organization}

### Headers to Include {#headers-to-include}

The library provides several main headers depending on your use case:

```cpp
// Core library (required)
#include <Modbus.h>           // Main interface and factory functions
#include <ModbusGlobal.h>     // Types, enums, constants, helper functions

// Client-side usage
#include <ModbusClient.h>     // High-level client wrapper
#include <ModbusClientPort.h> // Low-level client operations

// Server-side usage
#include <ModbusServerPort.h>    // Abstract server port interface
#include <ModbusTcpServer.h>     // TCP server implementation
#include <ModbusServerResource.h> // Single connection server resource

// Protocol-specific ports (if needed directly)
#include <ModbusTcpPort.h>    // TCP transport
#include <ModbusRtuPort.h>    // RTU serial transport
#include <ModbusAscPort.h>    // ASCII serial transport

// Qt integration (optional)
#include <ModbusQt.h>         // Qt-specific helpers and settings
#include <ModbusObject.h>     // Signal/slot mechanism
```

### Namespace {#namespace}

All types, functions, and classes are defined within the `Modbus` namespace:

```cpp
namespace Modbus {
    // All API elements here
}
```

---

## Global Types and Enumerations {#global-types-and-enumerations}

### Type Definitions {#type-definitions}

```cpp
namespace Modbus {

typedef void* Handle;              // Native OS handle type
typedef char Char;                 // Character type for strings
typedef uint32_t Timer;            // Timer value in milliseconds
typedef int64_t Timestamp;         // UNIX timestamp in milliseconds
typedef std::string String;        // Standard string type

template <class T>
using List = std::list<T>;         // Standard list container

}
```

### Protocol Types {#protocol-types}

```cpp
enum ProtocolType {
    ASC,  // ASCII version of Modbus (serial, hexadecimal encoding, LRC checksum)
    RTU,  // RTU version of Modbus (serial, binary encoding, CRC-16 checksum)
    TCP   // TCP/IP version of Modbus (Ethernet, MBAP header)
};
```

### Memory Types {#memory-types}

```cpp
enum MemoryType {
    Memory_Unknown = 0xFFFF,              // Invalid memory type
    Memory_0x = 0,                        // Coils/discrete outputs (read/write bits)
    Memory_Coils = Memory_0x,             // Alias for Memory_0x
    Memory_1x = 1,                        // Discrete inputs (read-only bits)
    Memory_DiscreteInputs = Memory_1x,    // Alias for Memory_1x
    Memory_3x = 3,                        // Input registers (read-only 16-bit)
    Memory_InputRegisters = Memory_3x,    // Alias for Memory_3x
    Memory_4x = 4,                        // Holding registers (read/write 16-bit)
    Memory_HoldingRegisters = Memory_4x   // Alias for Memory_4x
};
```

### Status Codes {#api-status-codes}

```cpp
enum StatusCode {
    // General status codes
    Status_Processing = 0x80000000,  // Operation in progress, call again
    Status_Good = 0x00000000,        // Success
    Status_Bad = 0x01000000,         // General error
    Status_Uncertain = 0x02000000,   // Undefined status
    
    // Modbus standard exception codes (0x01-0x0B)
    Status_BadIllegalFunction = Status_Bad | 0x01,                    // Function not supported
    Status_BadIllegalDataAddress = Status_Bad | 0x02,                 // Invalid data address
    Status_BadIllegalDataValue = Status_Bad | 0x03,                   // Invalid data value
    Status_BadServerDeviceFailure = Status_Bad | 0x04,                // Device failure
    Status_BadAcknowledge = Status_Bad | 0x05,                        // Long operation accepted
    Status_BadServerDeviceBusy = Status_Bad | 0x06,                   // Device busy
    Status_BadNegativeAcknowledge = Status_Bad | 0x07,                // Cannot perform function
    Status_BadMemoryParityError = Status_Bad | 0x08,                  // Memory parity error
    Status_BadGatewayPathUnavailable = Status_Bad | 0x0A,             // Gateway path unavailable
    Status_BadGatewayTargetDeviceFailedToRespond = Status_Bad | 0x0B, // Gateway target no response
    
    // Common protocol errors (0x101+)
    Status_BadEmptyResponse = Status_Bad | 0x101,      // Empty response
    Status_BadNotCorrectRequest,                        // Invalid request
    Status_BadNotCorrectResponse,                       // Invalid response
    Status_BadWriteBufferOverflow,                      // Write buffer overflow
    Status_BadReadBufferOverflow,                       // Read buffer overflow
    Status_BadPortClosed,                               // Port is closed
    
    // Serial port errors (0x201+)
    Status_BadSerialOpen = Status_Bad | 0x201,         // Cannot open serial port
    Status_BadSerialWrite,                              // Serial write error
    Status_BadSerialRead,                               // Serial read error
    Status_BadSerialReadTimeout,                        // Serial read timeout
    Status_BadSerialWriteTimeout,                       // Serial write timeout
    
    // ASCII protocol errors (0x301+)
    Status_BadAscMissColon = Status_Bad | 0x301,       // Missing ':' start character
    Status_BadAscMissCrLf,                              // Missing CR-LF end characters
    Status_BadAscChar,                                  // Invalid ASCII character
    Status_BadLrc,                                      // Invalid LRC checksum
    
    // RTU protocol errors (0x401+)
    Status_BadCrc = Status_Bad | 0x401,                // Invalid CRC checksum
    
    // TCP errors (0x501+)
    Status_BadTcpCreate = Status_Bad | 0x501,          // Cannot create TCP socket
    Status_BadTcpConnect,                               // Cannot connect TCP
    Status_BadTcpWrite,                                 // Cannot write TCP
    Status_BadTcpRead,                                  // Cannot read TCP
    Status_BadTcpBind,                                  // Cannot bind TCP socket
    Status_BadTcpListen,                                // Cannot listen TCP socket
    Status_BadTcpAccept,                                // Cannot accept TCP connection
    Status_BadTcpDisconnect                             // Bad disconnection
};
```

### Serial Port Configuration Types {#serial-port-configuration-types}

```cpp
enum Parity {
    NoParity,    // No parity bit (most common)
    EvenParity,  // Even parity
    OddParity,   // Odd parity (ensures state transition)
    SpaceParity, // Space parity (always 0)
    MarkParity   // Mark parity (always 1)
};

enum StopBits {
    OneStop,        // 1 stop bit (standard)
    OneAndHalfStop, // 1.5 stop bits
    TwoStop         // 2 stop bits
};

enum FlowControl {
    NoFlowControl,   // No flow control
    HardwareControl, // RTS/CTS hardware flow control
    SoftwareControl  // XON/XOFF software flow control
};
```

### Configuration Structures {#configuration-structures}

```cpp
// TCP connection settings
struct TcpSettings {
    union
    {    
    const Char *host;   // IP address or hostname to connect (client)
    const Char *ipaddr; // IP address to bind (server)
    };
    uint16_t port;       // TCP port number (default: 502)
    uint32_t timeout;    // Connection timeout in milliseconds
    uint32_t maxconn;    // Max simultaneous connections (server only)
};

// Serial port settings
struct SerialSettings {
    const Char *portName;         // Serial port name (e.g., "COM1", "/dev/ttyS0")
    int32_t baudRate;             // Baud rate (e.g., 9600, 19200, 115200)
    int8_t dataBits;              // Data bits (5, 6, 7, or 8)
    Parity parity;                // Parity setting
    StopBits stopBits;            // Stop bits setting
    FlowControl flowControl;      // Flow control setting
    uint32_t timeoutFirstByte;    // Timeout for first byte (ms)
    uint32_t timeoutInterByte;    // Inter-byte timeout (ms)
};
```

---

## Status Checking Functions {#status-checking-functions}

These inline functions provide convenient status checking:

```cpp
namespace Modbus {

// Check if operation is still in progress
inline bool StatusIsProcessing(StatusCode status);

// Check if operation succeeded
inline bool StatusIsGood(StatusCode status);

// Check if operation failed
inline bool StatusIsBad(StatusCode status);

// Check if status is uncertain
inline bool StatusIsUncertain(StatusCode status);

// Check if error is a standard Modbus exception (0x01-0x0B)
inline bool StatusIsStandardError(StatusCode status);

}
```

**Example:**
```cpp
Modbus::StatusCode status = client.readHoldingRegisters(1, 0, 10, buffer);
if (Modbus::StatusIsGood(status)) {
    // Success - process data
} else if (Modbus::StatusIsProcessing(status)) {
    // Still working - call again
} else {
    // Error occurred
    const char *errorText = client.lastPortErrorText();
}
```

---

## Helper Functions {#helper-functions}

### Bit Manipulation {#bit-manipulation}

```cpp
namespace Modbus {

// Get single bit from bit array
inline bool getBit(const void *bitBuff, uint16_t bitNum);
inline bool getBitS(const void *bitBuff, uint16_t bitNum, uint16_t maxBitCount);

// Set single bit in bit array
inline void setBit(void *bitBuff, uint16_t bitNum, bool value);
inline void setBitS(void *bitBuff, uint16_t bitNum, bool value, uint16_t maxBitCount);

// Get multiple bits from bit array to bool array
inline bool* getBits(const void *bitBuff, uint16_t bitNum, uint16_t bitCount, bool *boolBuff);
inline bool* getBitsS(const void *bitBuff, uint16_t bitNum, uint16_t bitCount, bool *boolBuff, uint16_t maxBitCount);

// Set multiple bits from bool array to bit array
inline void* setBits(void *bitBuff, uint16_t bitNum, uint16_t bitCount, const bool *boolBuff);
inline void* setBitsS(void *bitBuff, uint16_t bitNum, uint16_t bitCount, const bool *boolBuff, uint16_t maxBitCount);

}
```

### Memory Operations {#memory-operations}

```cpp
namespace Modbus {

// Read/write 16-bit registers
StatusCode readMemRegs(uint32_t offset, uint32_t count, void *values, 
                       const void *memBuff, uint32_t memRegCount, uint32_t *outCount = nullptr);
                       
StatusCode writeMemRegs(uint32_t offset, uint32_t count, const void *values, 
                        void *memBuff, uint32_t memRegCount, uint32_t *outCount = nullptr);

// Read/write discrete bits
StatusCode readMemBits(uint32_t offset, uint32_t count, void *values, 
                       const void *memBuff, uint32_t memBitCount, uint32_t *outCount = nullptr);
                       
StatusCode writeMemBits(uint32_t offset, uint32_t count, const void *values, 
                        void *memBuff, uint32_t memBitCount, uint32_t *outCount = nullptr);

}
```

### Checksums {#checksums}

```cpp
namespace Modbus {

// Calculate CRC-16 checksum for RTU protocol
uint16_t crc16(const uint8_t *byteArr, uint32_t count);

// Calculate LRC checksum for ASCII protocol
uint8_t lrc(const uint8_t *byteArr, uint32_t count);

}
```

### String Conversion {#string-conversion}

```cpp
namespace Modbus {

// Convert bytes to/from ASCII representation
uint32_t bytesToAscii(const uint8_t* bytesBuff, uint8_t* asciiBuff, uint32_t count);
uint32_t asciiToBytes(const uint8_t* asciiBuff, uint8_t* bytesBuff, uint32_t count);

// Format bytes as string
Char* sbytes(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen);
String bytesToString(const uint8_t* buff, uint32_t count);

// Format ASCII as string
Char* sascii(const uint8_t* buff, uint32_t count, Char *str, uint32_t strmaxlen);
String asciiToString(const uint8_t* buff, uint32_t count);

// Enum to string conversions
const Char* sprotocolType(ProtocolType type);
const Char* sbaudRate(int32_t baudRate);
const Char* sdataBits(int8_t dataBits);
const Char* sparity(Parity parity);
const Char* sstopBits(StopBits stopBits);
const Char* sflowControl(FlowControl flowControl);

// String to enum conversions
ProtocolType toprotocolType(const Char *s);
int32_t tobaudRate(const Char *s);
int8_t todataBits(const Char *s);
Parity toparity(const Char *s);
StopBits tostopBits(const Char *s);
FlowControl toflowControl(const Char *s);

}
```

### System Functions {#system-functions}

```cpp
namespace Modbus {

// Get library version
uint32_t modbusLibVersion();        // Returns (major << 16) | (minor << 8) | patch
const Char* modbusLibVersionStr();  // Returns "major.minor.patch"

// Time functions
Timer timer();                      // Get timer value in milliseconds
Timestamp currentTimestamp();       // Get UNIX timestamp in milliseconds
void msleep(uint32_t msec);         // Sleep for specified milliseconds

// Console output
void setConsoleColor(Color color);  // Set console text color

// Serial port enumeration
List<String> availableSerialPorts();     // Get list of available serial ports
List<int32_t> availableBaudRate();       // Get list of standard baud rates
List<int8_t> availableDataBits();        // Get list of data bits options
List<Parity> availableParity();          // Get list of parity options
List<StopBits> availableStopBits();      // Get list of stop bits options
List<FlowControl> availableFlowControl(); // Get list of flow control options

}
```

---

## ModbusInterface (Abstract Device Interface) {#modbusinterface-abstract-device-interface}

`ModbusInterface` defines the standard Modbus function interface that devices must implement.

### Class Declaration {#api-modbusinterface-decl}

```cpp
class ModbusInterface {
public:
    // FC 01: Read Coils (0x bits)
    virtual StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values);
    
    // FC 02: Read Discrete Inputs (1x bits)
    virtual StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values);
    
    // FC 03: Read Holding Registers (4x regs)
    virtual StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    
    // FC 04: Read Input Registers (3x regs)
    virtual StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    
    // FC 05: Write Single Coil
    virtual StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value);
    
    // FC 06: Write Single Register
    virtual StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value);
    
    // FC 07: Read Exception Status
    virtual StatusCode readExceptionStatus(uint8_t unit, uint8_t *status);
    
    // FC 08: Diagnostics
    virtual StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, 
                                   const void *indata, uint8_t *outsize, void *outdata);
    
    // FC 11: Get Comm Event Counter
    virtual StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount);
    
    // FC 12: Get Comm Event Log
    virtual StatusCode getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, 
                                       uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
    
    // FC 15: Write Multiple Coils
    virtual StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values);
    
    // FC 16: Write Multiple Registers
    virtual StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
    
    // FC 17: Report Server ID
    virtual StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data);
    
    // FC 22: Mask Write Register
    virtual StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
    
    // FC 23: Read/Write Multiple Registers
    virtual StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, 
                                                  uint16_t *readValues, uint16_t writeOffset, 
                                                  uint16_t writeCount, const uint16_t *writeValues);
    
    // FC 24: Read FIFO Queue
    virtual StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
};
```

**Note:** Default implementations return `Status_BadIllegalFunction`. Override only the functions your device supports.

---

## ModbusPort (Abstract Base Class) {#modbusport-abstract-base-class}

Abstract base class for all protocol-specific port implementations.

### Class Declaration {#api-modbusport-decl}

```cpp
class ModbusPort {
public:
    virtual ~ModbusPort();
    
    // Protocol information
    virtual ProtocolType type() const = 0;
    virtual Handle handle() const = 0;
    
    // Connection management
    virtual StatusCode open() = 0;
    virtual StatusCode close() = 0;
    virtual bool isOpen() const = 0;
    
    // Port state
    bool isChanged() const;
    bool isServerMode() const;
    virtual void setServerMode(bool mode);
    bool isBlocking() const;
    bool isNonBlocking() const;
    
    // Configuration
    uint32_t timeout() const;
    void setTimeout(uint32_t timeout);
    virtual void setNextRequestRepeated(bool v);
    
    // Error information
    StatusCode lastErrorStatus() const;
    const Char* lastErrorText() const;
    
    // Buffer access (for advanced usage)
    virtual const uint8_t* readBufferData() const = 0;
    virtual uint16_t readBufferSize() const = 0;
    virtual const uint8_t* writeBufferData() const = 0;
    virtual uint16_t writeBufferSize() const = 0;
    
protected:
    // Low-level I/O (implemented by derived classes)
    virtual StatusCode writeBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff) = 0;
    virtual StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) = 0;
    virtual StatusCode write() = 0;
    virtual StatusCode read() = 0;
};
```

---

## ModbusTcpPort {#modbustcpport}

TCP/IP transport implementation for Modbus.

### Class Declaration {#api-modbustcpport-decl}

```cpp
class ModbusTcpPort : public ModbusPort {
public:
    // Default values
    struct Defaults {
        const Char *host;      // Default: "127.0.0.1"
        const uint16_t port;   // Default: 502
        const uint32_t timeout; // Default: 3000 ms
        
        Defaults();
        static const Defaults& instance();
    };
    
    // Constructors
    ModbusTcpPort(bool blocking = false);
    ModbusTcpPort(ModbusTcpSocket *socket, bool blocking = false);
    ~ModbusTcpPort();
    
    // Protocol information
    ProtocolType type() const override { return TCP; }
    Handle handle() const override;
    
    // Connection management
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    
    // Configuration
    const Char* host() const;
    void setHost(const Char *host);
    uint16_t port() const;
    void setPort(uint16_t port);
    
    // Transaction management
    void setNextRequestRepeated(bool v) override;
    bool autoIncrement() const;
    uint16_t transactionId() const;
    
    // Buffer access
    const uint8_t* readBufferData() const override;
    uint16_t readBufferSize() const override;
    const uint8_t* writeBufferData() const override;
    uint16_t writeBufferSize() const override;
};
```

### Example {#api-tcpport-example}

```cpp
// Create and configure TCP port
ModbusTcpPort port(false);  // Non-blocking mode
port.setHost("192.168.1.100");
port.setPort(502);
port.setTimeout(5000);  // 5 seconds

// Open connection
StatusCode status = port.open();
while (StatusIsProcessing(status)) {
    status = port.open();  // Continue until complete
}

if (StatusIsGood(status)) {
    // Connection established
}
```

---

## ModbusSerialPort (Abstract Base Class) {#modbusserialport-abstract-base-class}

Base class for serial port implementations (RTU and ASCII).

### Class Declaration {#api-modbusserialport-decl}

```cpp
class ModbusSerialPort : public ModbusPort {
public:
    // Default values
    struct Defaults {
        const Char *portName;          // Default: "" (empty)
        const int32_t baudRate;        // Default: 9600
        const int8_t dataBits;         // Default: 8
        const Parity parity;           // Default: NoParity
        const StopBits stopBits;       // Default: OneStop
        const FlowControl flowControl; // Default: NoFlowControl
        const uint32_t timeoutFirstByte; // Default: 1000 ms
        const uint32_t timeoutInterByte; // Default: 100 ms
        
        Defaults();
        static const Defaults& instance();
    };
    
    virtual ~ModbusSerialPort();
    
    // Connection management
    Handle handle() const override;
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    
    // Serial port configuration
    const Char* portName() const;
    void setPortName(const Char *portName);
    int32_t baudRate() const;
    void setBaudRate(int32_t baudRate);
    int8_t dataBits() const;
    void setDataBits(int8_t dataBits);
    Parity parity() const;
    void setParity(Parity parity);
    StopBits stopBits() const;
    void setStopBits(StopBits stopBits);
    FlowControl flowControl() const;
    void setFlowControl(FlowControl flowControl);
    
    // Timeouts
    uint32_t timeoutFirstByte() const;
    void setTimeoutFirstByte(uint32_t timeout);
    uint32_t timeoutInterByte() const;
    void setTimeoutInterByte(uint32_t timeout);
    
    // Buffer access
    const uint8_t* readBufferData() const override;
    uint16_t readBufferSize() const override;
    const uint8_t* writeBufferData() const override;
    uint16_t writeBufferSize() const override;
};
```

---

## ModbusRtuPort {#modbusrtuport}

RTU (binary) serial protocol implementation.

### Class Declaration {#api-modbusrtuport-decl}

```cpp
class ModbusRtuPort : public ModbusSerialPort {
public:
    ModbusRtuPort(bool blocking = false);
    ~ModbusRtuPort();
    
    ProtocolType type() const override { return RTU; }
};
```

### Example {#api-rtuport-example}

```cpp
// Create and configure RTU port
ModbusRtuPort port(false);  // Non-blocking mode
port.setPortName("COM1");   // Windows: "COM1", Linux: "/dev/ttyS0"
port.setBaudRate(9600);
port.setDataBits(8);
port.setParity(Modbus::NoParity);
port.setStopBits(Modbus::OneStop);
port.setTimeoutFirstByte(1000);
port.setTimeoutInterByte(100);

StatusCode status = port.open();
if (StatusIsGood(status)) {
    // Port opened successfully
}
```

---

## ModbusAscPort {#modbusascport}

ASCII (hexadecimal text) serial protocol implementation.

### Class Declaration {#api-modbusascport-decl}

```cpp
class ModbusAscPort : public ModbusSerialPort {
public:
    ModbusAscPort(bool blocking = false);
    ~ModbusAscPort();
    
    ProtocolType type() const override { return ASC; }
};
```

### Example {#api-ascport-example}

```cpp
// Create and configure ASCII port
ModbusAscPort port(false);
port.setPortName("/dev/ttyUSB0");
port.setBaudRate(9600);
port.setDataBits(7);  // Often 7 bits for ASCII
port.setParity(Modbus::EvenParity);
port.setStopBits(Modbus::OneStop);

StatusCode status = port.open();
```

---

## ModbusClientPort {#api-modbusclientport}

Client-side protocol implementation with automatic resource sharing.

### Class Declaration {#api-modbusclientport-decl}

```cpp
class ModbusClientPort : public ModbusObject, public ModbusInterface {
public:
    enum RequestStatus {
        Enable,   // Request can be made
        Disable,  // Port busy with another client
        Process   // Current client is processing
    };
    
    // Constructor
    ModbusClientPort(ModbusPort *port);
    
    // Protocol information
    ProtocolType type() const;
    ModbusPort* port() const;
    void setPort(ModbusPort *port);
    
    // Connection management
    StatusCode close();
    bool isOpen() const;
    
    // Configuration
    uint32_t tries() const;
    void setTries(uint32_t v);
    bool isBroadcastEnabled() const;
    void setBroadcastEnabled(bool enable);
    
    // Status information
    StatusCode lastStatus() const;
    Timestamp lastStatusTimestamp() const;
    StatusCode lastErrorStatus() const;
    const Char* lastErrorText() const;
    uint32_t lastTries() const;
    
    // Multi-client resource management
    const ModbusObject* currentClient() const;
    RequestStatus getRequestStatus(ModbusObject *client);
    void cancelRequest(ModbusObject *client);
    
    // ModbusInterface methods (single-threaded usage)
    StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values) override;
    StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override;
    StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value) override;
    StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value) override;
    StatusCode readExceptionStatus(uint8_t unit, uint8_t *value) override;
    StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata) override;
    StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, uint16_t *eventCount) override;
    StatusCode getCommEventLog(uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff) override;
    StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values) override;
    StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) override;
    StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data) override;
    StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask) override;
    StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues) override;
    StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values) override;
    
    // Bool array variants
    StatusCode readCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);
    StatusCode readDiscreteInputsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, bool *values);
    StatusCode writeMultipleCoilsAsBoolArray(uint8_t unit, uint16_t offset, uint16_t count, const bool *values);
    
    // Multi-client methods (pass ModbusObject* as first parameter)
    StatusCode readCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
    StatusCode readDiscreteInputs(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, void *values);
    StatusCode readHoldingRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    StatusCode readInputRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
    StatusCode writeSingleCoil(ModbusObject *client, uint8_t unit, uint16_t offset, bool value);
    StatusCode writeSingleRegister(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t value);
    StatusCode writeMultipleCoils(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const void *values);
    StatusCode writeMultipleRegisters(ModbusObject *client, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
    // ... (other functions with client parameter)
    
    // Signals (connect callbacks to these events)
    void signalOpened(const Char *source);
    void signalClosed(const Char *source);
    void signalTx(const Char *source, const uint8_t* buff, uint16_t size);
    void signalRx(const Char *source, const uint8_t* buff, uint16_t size);
    void signalError(const Char *source, StatusCode status, const Char *text);
};
```

### Example: Single Client {#example-single-client}

```cpp
// Create port and client port
ModbusTcpPort *tcpPort = new ModbusTcpPort(false);
tcpPort->setHost("192.168.1.100");
ModbusClientPort clientPort(tcpPort);

clientPort.setTries(3);  // Retry failed requests 3 times

// Open connection
StatusCode status = tcpPort->open();
while (StatusIsProcessing(status)) {
    status = tcpPort->open();
}

// Read holding registers
uint16_t regs[10];
status = clientPort.readHoldingRegisters(1, 0, 10, regs);
while (StatusIsProcessing(status)) {
    status = clientPort.readHoldingRegisters(1, 0, 10, regs);
}

if (StatusIsGood(status)) {
    // Process regs data
}
```

---

## ModbusClient {#api-modbusclient}

High-level client wrapper for specific device communication.

### Settings Overview {#api-tcpserver-settings}

### Class Declaration {#api-modbusclient-decl}
```cpp
class ModbusClient : public ModbusObject {
public:
    // Constructor
    ModbusClient(uint8_t unit, ModbusClientPort *port);
    
    // Device information
    ProtocolType type() const;
    uint8_t unit() const;
    void setUnit(uint8_t unit);
    bool isOpen() const;
    ModbusClientPort* port() const;
    
    // Modbus functions (unit parameter omitted, set in constructor)
    StatusCode readCoils(uint16_t offset, uint16_t count, void *values);
    StatusCode readDiscreteInputs(uint16_t offset, uint16_t count, void *values);
    StatusCode readHoldingRegisters(uint16_t offset, uint16_t count, uint16_t *values);
    StatusCode readInputRegisters(uint16_t offset, uint16_t count, uint16_t *values);
    StatusCode writeSingleCoil(uint16_t offset, bool value);
    StatusCode writeSingleRegister(uint16_t offset, uint16_t value);
    StatusCode readExceptionStatus(uint8_t *value);
    StatusCode diagnostics(uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata);
    StatusCode getCommEventCounter(uint16_t *status, uint16_t *eventCount);
### Usage Examples {#api-tcpserver-examples}

    StatusCode getCommEventLog(uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff);
    StatusCode writeMultipleCoils(uint16_t offset, uint16_t count, const void *values);
    StatusCode writeMultipleRegisters(uint16_t offset, uint16_t count, const uint16_t *values);
    StatusCode reportServerID(uint8_t *count, uint8_t *data);
    StatusCode maskWriteRegister(uint16_t offset, uint16_t andMask, uint16_t orMask);
    StatusCode readWriteMultipleRegisters(uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
    StatusCode readFIFOQueue(uint16_t fifoadr, uint16_t *count, uint16_t *values);
    
    // Bool array variants
    StatusCode readCoilsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
    StatusCode readDiscreteInputsAsBoolArray(uint16_t offset, uint16_t count, bool *values);
    StatusCode writeMultipleCoilsAsBoolArray(uint16_t offset, uint16_t count, const bool *values);
    
    // Status information
    StatusCode lastPortStatus() const;
    StatusCode lastPortErrorStatus() const;
    const Char* lastPortErrorText() const;
};
```

### Example: Multiple Clients {#example-multiple-clients}

```cpp
// Create shared port
ModbusTcpPort *tcpPort = new ModbusTcpPort(false);
tcpPort->setHost("192.168.1.100");
ModbusClientPort *clientPort = new ModbusClientPort(tcpPort);

// Create multiple clients for different devices
ModbusClient device1(1, clientPort);  // Unit address 1
ModbusClient device2(2, clientPort);  // Unit address 2
ModbusClient device3(3, clientPort);  // Unit address 3

tcpPort->open();

// Main loop - clients automatically share the port
while (running) {
    uint16_t regs1[10], regs2[10], regs3[10];
    
    StatusCode s1 = device1.readHoldingRegisters(0, 10, regs1);
    StatusCode s2 = device2.readHoldingRegisters(0, 10, regs2);
    StatusCode s3 = device3.readHoldingRegisters(0, 10, regs3);
    
    // Process results...
    
    Modbus::msleep(10);
}
```

---

## ModbusServerPort (Abstract Base Class) {#modbusserverport-abstract-base-class}

Abstract base class for server-side port implementations.

### Class Declaration {#api-modbusserverport-decl}

```cpp
class ModbusServerPort : public ModbusObject {
public:
    // Device interface
    ModbusInterface* device() const;
    void setDevice(ModbusInterface *device);
    
    // Server port interface
    virtual ProtocolType type() const = 0;
    virtual bool isTcpServer() const;
    virtual StatusCode open() = 0;
    virtual StatusCode close() = 0;
    virtual bool isOpen() const = 0;
    virtual uint32_t timeout() const = 0;
    virtual void setTimeout(uint32_t timeout) = 0;
    
    // Configuration
    bool isBroadcastEnabled() const;
    virtual void setBroadcastEnabled(bool enable);
    const void\* unitMap() const;
    virtual void setUnitMap(const void \*unitmap);
    void\* context() const;
    void setContext(void \*context);
    
    // Main processing
    virtual StatusCode process() = 0;
    
    // State checking
    bool isStateClosed() const;
    
    // Signals
    void signalOpened(const Char *source);
    void signalClosed(const Char *source);
    void signalTx(const Char *source, const uint8_t* buff, uint16_t size);
    void signalRx(const Char *source, const uint8_t* buff, uint16_t size);
    void signalError(const Char *source, StatusCode status, const Char *text);
};
```

---

## ModbusServerResource {#api-modbusserverresource}

Single-connection server implementation wrapping any ModbusPort.

### Class Declaration {#api-modbusserverresource-decl}

```cpp
class ModbusServerResource : public ModbusServerPort {
public:
    // Constructor
    ModbusServerResource(ModbusPort *port, ModbusInterface *device);
    
    // Port access
    ModbusPort* port() const;
    
    // Server port interface
    ProtocolType type() const override;
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    uint32_t timeout() const override;
    void setTimeout(uint32_t timeout) override;
    StatusCode process() override;
};
```

### Example: RTU Server {#example-rtu-server}

```cpp
// Implement device interface
class MyDevice : public ModbusInterface {
public:
    StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values) override {
        // Read from device memory
        for (uint16_t i = 0; i < count; i++) {
            values[i] = m_registers[offset + i];
        }
        return Status_Good;
    }
    
    StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values) override {
        // Write to device memory
        for (uint16_t i = 0; i < count; i++) {
            m_registers[offset + i] = values[i];
        }
        return Status_Good;
    }
    
private:
    uint16_t m_registers[1000];
};

// Create server
MyDevice device;
ModbusRtuPort *rtuPort = new ModbusRtuPort(false);
rtuPort->setPortName("COM1");
rtuPort->setBaudRate(9600);

ModbusServerResource server(rtuPort, &device);
server.open();

// Main loop
while (running) {
    server.process();  // Handle incoming requests
}

server.close();
```

---

## ModbusTcpServer {#api-modbustcpserver}

Multi-connection TCP server implementation.

### Class Declaration {#api-modbustcpserver-decl}

```cpp
class ModbusTcpServer : public ModbusServerPort {
public:
    // Default values
    struct Defaults {
        const Char*    ipaddr ; // Default: "0.0.0.0"
        const uint16_t port   ; // Default: 502
        const uint32_t timeout; // Default: 3000 ms
        const uint32_t maxconn; // Default: 10
        
        Defaults();
        static const Defaults& instance();
    };
    
    // Constructor
    ModbusTcpServer(ModbusInterface *device);
    ~ModbusTcpServer();
    
    // Configuration
    const Char* ipaddr() const;     // Bind address (e.g., "127.0.0.1", "0.0.0.0")
    void setIpaddr(const Char *ip); // Set bind address
    uint16_t port() const;
    void setPort(uint16_t port);
    uint32_t timeout() const override;
    void setTimeout(uint32_t timeout) override;
    uint32_t maxConnections() const;
    void setMaxConnections(uint32_t maxconn);
    
    // Server interface
    ProtocolType type() const override { return TCP; }
    bool isTcpServer() const override { return true; }
    StatusCode open() override;
    StatusCode close() override;
    bool isOpen() const override;
    void setBroadcastEnabled(bool enable) override;
    void setUnitMap(const void *unitmap) override;
    StatusCode process() override;
    
    // Customization points (override in derived classes)
    virtual ModbusServerPort* createTcpPort(ModbusTcpSocket *socket);
    virtual void deleteTcpPort(ModbusServerPort *port);
    
    // Additional signals
    void signalNewConnection(const Char *source);
    void signalCloseConnection(const Char *source);
};
```

### Example: TCP Server {#example-tcp-server}

```cpp
// Implement device interface
class MyDevice : public ModbusInterface {
    // ... implement required methods
};

// Create and configure server
MyDevice device;
ModbusTcpServer server(&device);
server.setIpaddr("127.0.0.1");
server.setPort(502);
server.setTimeout(5000);
server.setMaxConnections(10);

// Open server
StatusCode status = server.open();
while (StatusIsProcessing(status)) {
    status = server.open();
}

if (StatusIsGood(status)) {
    // Main server loop
    while (running) {
        server.process();  // Handle all connections
        Modbus::msleep(1);
    }
    
    server.close();
}
```

---

## ModbusObject (Signal/Slot System) {#modbusobject-signal-slot-system}

Base class providing simplified signal/slot mechanism for event handling.

### Class Declaration {#api-modbusobject-decl}

```cpp
class ModbusObject {
public:
    // Get current signal sender (valid only within slot callback)
    static ModbusObject* sender();
    
    // Constructor and destructor
    ModbusObject();
    virtual ~ModbusObject();
    
    // Object naming
    const Char* objectName() const;
    void setObjectName(const Char *name);
    
    // Connect signal to slot (method)
    template <class SignalClass, class T, class ReturnType, class... Args>
    void connect(ModbusMethodPointer<SignalClass, ReturnType, Args...> signalMethodPtr,
                 T *object,
                 ModbusMethodPointer<T, ReturnType, Args...> objectMethodPtr);
    
    // Connect signal to slot (function)
    template <class SignalClass, class ReturnType, class... Args>
    void connect(ModbusMethodPointer<SignalClass, ReturnType, Args...> signalMethodPtr,
                 ModbusFunctionPointer<ReturnType, Args...> funcPtr);
    
    // Disconnect slots
    template \<class ReturnType, class... Args\>
    void disconnect(ModbusFunctionPointer\<ReturnType, Args...\> funcPtr);
    
    template \<class T, class ReturnType, class... Args\>
    void disconnect(T \*object, ModbusMethodPointer\<T, ReturnType, Args...\> objectMethodPtr);
    
    template \<class T\>
    void disconnect(T \*object);
};
```

### Signal/Slot Example {#signal-slot-example}

```cpp
// Callback function
void onDataReceived(const char *source, const uint8_t *buff, uint16_t size) {
    printf("Received from %s: %d bytes\n", source, size);
}

// Callback method
class MyHandler {
public:
    void onError(const char *source, Modbus::StatusCode status, const char *text) {
        printf("Error from %s: %s\n", source, text);
    }
};

// Usage
ModbusClientPort clientPort(port);
MyHandler handler;

// Connect signals
clientPort.connect(&ModbusClientPort::signalRx, &onDataReceived);
clientPort.connect(&ModbusClientPort::signalError, &handler, &MyHandler::onError);

// Later: disconnect
clientPort.disconnect(&onDataReceived);
clientPort.disconnect(&handler);
```

---

## Factory Functions {#factory-functions}

Convenience functions for creating port objects.

```cpp
namespace Modbus {

// Create generic port
ModbusPort* createPort(ProtocolType type, const void *settings, bool blocking);

// Create client port
ModbusClientPort* createClientPort(ProtocolType type, const void *settings, bool blocking);

// Create server port
ModbusServerPort* createServerPort(ModbusInterface *device, ProtocolType type, const void *settings, bool blocking);

}
```

### Example {#example}

```cpp
// Create TCP client port
Modbus::TcpSettings tcpSettings;
tcpSettings.host = "192.168.1.100";
tcpSettings.port = 502;
tcpSettings.timeout = 5000;

ModbusClientPort *client = Modbus::createClientPort(Modbus::TCP, &tcpSettings, false);

// Create RTU server port
Modbus::SerialSettings serialSettings;
serialSettings.portName = "COM1";
serialSettings.baudRate = 9600;
serialSettings.dataBits = 8;
serialSettings.parity = Modbus::NoParity;
serialSettings.stopBits = Modbus::OneStop;
serialSettings.flowControl = Modbus::NoFlowControl;
serialSettings.timeoutFirstByte = 1000;
serialSettings.timeoutInterByte = 100;

MyDevice device;
ModbusServerPort *server = Modbus::createServerPort(&device, Modbus::RTU, &serialSettings, false);
```

---

## Qt Integration (Optional) {#qt-integration-optional}

When compiled with Qt support, additional Qt-specific features are available.

### Qt Types {#qt-types}

```cpp
namespace Modbus {

typedef QHash<QString, QVariant> Settings;  // Settings map for Qt

// Settings keys
class Strings {
public:
    const QString unit;
    const QString type;
    const QString tries;
    const QString host;
    const QString port;
    const QString timeout;
    const QString maxconn;
    const QString serialPortName;
    const QString baudRate;
    const QString dataBits;
    const QString parity;
    const QString stopBits;
    const QString flowControl;
    const QString timeoutFirstByte;
    const QString timeoutInterByte;
    const QString isBroadcastEnabled;
    // ... (parity, stopBits, flowControl enum value strings)
    
    static const Strings& instance();
};

// Default values for Qt
class Defaults {
public:
    const uint8_t unit;
    const ProtocolType type;
    const uint32_t tries;
    const QString host;
    const uint16_t port;
    const uint32_t timeout;
    const uint32_t maxconn;
    const QString serialPortName;
    const int32_t baudRate;
    const int8_t dataBits;
    const Parity parity;
    const StopBits stopBits;
    const FlowControl flowControl;
    const uint32_t timeoutFirstByte;
    const uint32_t timeoutInterByte;
    const bool isBroadcastEnabled;
    
    static const Defaults& instance();
};

}
```

### Qt Settings Functions {#qt-settings-functions}

```cpp
namespace Modbus {

// Get settings (returns default if not found or conversion fails)
uint8_t getSettingUnit(const Settings &s, bool *ok = nullptr);
ProtocolType getSettingType(const Settings &s, bool *ok = nullptr);
uint32_t getSettingTries(const Settings &s, bool *ok = nullptr);
QString getSettingHost(const Settings &s, bool *ok = nullptr);
uint16_t getSettingPort(const Settings &s, bool *ok = nullptr);
uint32_t getSettingTimeout(const Settings &s, bool *ok = nullptr);
// ... (other getters)

// Set settings
void setSettingUnit(Settings &s, uint8_t v);
void setSettingType(Settings &s, ProtocolType v);
void setSettingTries(Settings &s, uint32_t v);
void setSettingHost(Settings &s, const QString &v);
void setSettingPort(Settings &s, uint16_t v);
void setSettingTimeout(Settings &s, uint32_t v);
// ... (other setters)

}
```

### Qt Factory Functions {#qt-factory-functions}

```cpp
namespace Modbus {

// Create port from Qt settings
ModbusPort* createPort(const Settings &settings, bool blocking = false);
ModbusClientPort* createClientPort(const Settings &settings, bool blocking = false);
ModbusServerPort* createServerPort(ModbusInterface *device, const Settings &settings, bool blocking = false);

}
```

### Qt Example {#qt-example}

```cpp
// Configure using Qt settings
Modbus::Settings settings;
const Modbus::Strings &keys = Modbus::Strings::instance();

settings[keys.type] = "TCP";
settings[keys.host] = "192.168.1.100";
settings[keys.port] = 502;
settings[keys.timeout] = 5000;
settings[keys.tries] = 3;

// Create client from settings
ModbusClientPort *client = Modbus::createClientPort(settings, false);
```

---

## Address Class (Optional) {#address-class-optional}

When `MB_ADDRESS_CLASS_DISABLE` is not defined, the `Address` class provides Modbus address notation support.

### Class Declaration {#class-declaration}

```cpp
namespace Modbus {

class Address {
public:
    enum Notation {
        Notation_Default,     // Same as Modbus notation
        Notation_Modbus,      // Standard: 000001, 100001, 300001, 400001
        Notation_IEC61131,    // IEC-61131: %Q0, %I0, %IW0, %MW0
        Notation_IEC61131Hex  // IEC-61131 Hex: %Q0h, %I0h, %IW0h, %MW0h
    };
    
    // Constructors
    Address();
    Address(MemoryType type, uint16_t offset);
    Address(uint32_t adr);
    
    // Properties
    bool isValid() const;
    MemoryType type() const;
    uint16_t offset() const;
    void setOffset(uint16_t offset);
    uint32_t number() const;
    void setNumber(uint16_t number);
    
    // Conversion
    int toInt() const;
    operator uint32_t() const;
    Address& operator=(uint32_t v);
    Address& operator+=(uint16_t c);
    
    // String conversion
    template<class StringT>
    static Address fromString(const StringT &s);
    
    template<class StringT>
    StringT toString(Notation notation) const;
};

}
```

### Address Examples {#address-examples}

```cpp
// Create addresses
Modbus::Address coil(Modbus::Memory_0x, 99);       // Offset 99
Modbus::Address holding(400001);                    // 400001 = 4x memory, offset 0
Modbus::Address input = Modbus::Address::fromString<std::string>("300123");

// Convert to string
std::string s1 = holding.toString<std::string>(Modbus::Address::Notation_Modbus);     // "400001"
std::string s2 = holding.toString<std::string>(Modbus::Address::Notation_IEC61131);   // "%MW0"
std::string s3 = holding.toString<std::string>(Modbus::Address::Notation_IEC61131Hex); // "%MW0000h"

// Arithmetic
Modbus::Address next = holding;
next += 10;  // Advance by 10 registers
```

---

## Constants and Limits {#constants-and-limits}

```cpp
namespace Modbus {

enum Constants {
    VALID_MODBUS_ADDRESS_BEGIN = 1,    // First valid unit address
    VALID_MODBUS_ADDRESS_END = 247,    // Last valid unit address
    STANDARD_TCP_PORT = 502            // Standard Modbus TCP port
};

// Protocol limits
`MB_MAX_BYTES` (255) - Max bytes in single request
`MB_MAX_REGISTERS` (127) - Max registers in single request (255/2)
`MB_MAX_DISCRETS` (2040) - Max discretes in single request (255*8)

// Modbus function codes
| Constant | Value | Description |
|----------|-------|-------------|
| `MBF_READ_COILS` | 1 | Read coils |
| `MBF_READ_DISCRETE_INPUTS` | 2 | Read discrete inputs |
| `MBF_READ_HOLDING_REGISTERS` | 3 | Read holding registers |
| `MBF_READ_INPUT_REGISTERS` | 4 | Read input registers |
| `MBF_WRITE_SINGLE_COIL` | 5 | Write single coil |
| `MBF_WRITE_SINGLE_REGISTER` | 6 | Write single register |
| `MBF_READ_EXCEPTION_STATUS` | 7 | Read exception status |
| `MBF_DIAGNOSTICS` | 8 | Diagnostics |
| `MBF_GET_COMM_EVENT_COUNTER` | 11 | Get comm event counter |
| `MBF_GET_COMM_EVENT_LOG` | 12 | Get comm event log |
| `MBF_WRITE_MULTIPLE_COILS` | 15 | Write multiple coils |
| `MBF_WRITE_MULTIPLE_REGISTERS` | 16 | Write multiple registers |
| `MBF_REPORT_SERVER_ID` | 17 | Report server ID |
| `MBF_MASK_WRITE_REGISTER` | 22 | Mask write register |
| `MBF_READ_WRITE_MULTIPLE_REGISTERS` | 23 | Read/write multiple registers |
| `MBF_READ_FIFO_QUEUE` | 24 | Read FIFO queue |

}
```

---

## Version Information {#version-information}

```cpp
// Library version macros
#define MODBUSLIB_VERSION ((MAJOR<<16)|(MINOR<<8)|(PATCH))
#define MODBUSLIB_VERSION_STR "major.minor.patch"

namespace Modbus {

// Version functions
uint32_t modbusLibVersion();         // Returns version as integer
const Char* modbusLibVersionStr();   // Returns version as string

}
```

**Example:**
```cpp
uint32_t version = Modbus::modbusLibVersion();
int major = (version >> 16) & 0xFF;
int minor = (version >> 8) & 0xFF;
int patch = version & 0xFF;

printf("ModbusLib version: %s\n", Modbus::modbusLibVersionStr());
```

---

## Error Handling Patterns {#api-error-patterns}

### Blocking Mode {#api-blocking-mode}

```cpp
ModbusTcpPort port(true);  // Blocking mode
port.setHost("192.168.1.100");

StatusCode status = port.open();  // Blocks until complete
if (StatusIsGood(status)) {
    // Connection established
} else {
    // Error occurred
    printf("Error: %s\n", port.lastErrorText());
}
```

### Non-Blocking Mode {#api-non-blocking-mode}

```cpp
ModbusTcpPort port(false);  // Non-blocking mode
port.setHost("192.168.1.100");

StatusCode status = port.open();
while (StatusIsProcessing(status)) {
    // Do other work while connecting...
    Modbus::msleep(10);
    status = port.open();  // Continue operation
}

if (StatusIsGood(status)) {
    // Connection established
} else {
    // Error occurred
    printf("Error: %s\n", port.lastErrorText());
}
```

### Retry Pattern {#retry-pattern}

```cpp
ModbusClientPort clientPort(port);
clientPort.setTries(3);  // Auto-retry up to 3 times

StatusCode status;
do {
    status = clientPort.readHoldingRegisters(1, 0, 10, buffer);
    if (StatusIsProcessing(status)) {
        Modbus::msleep(1);
    }
} while (StatusIsProcessing(status));

if (StatusIsGood(status)) {
    // Success
    uint32_t tries = clientPort.lastTries();
    printf("Succeeded after %u tries\n", tries);
} else {
    // Failed after all retries
    printf("Error: %s\n", clientPort.lastErrorText());
}
```

---

## Thread Safety Notes {#thread-safety-notes}

- **ModbusPort** and derived classes are **NOT thread-safe**. Each port instance should be accessed from a single thread only.
- **ModbusClientPort** supports multiple `ModbusClient` objects sharing one port through automatic resource arbitration, but all operations must be from the same thread.
- **ModbusObject** signal/slot mechanism is **NOT thread-safe**. Signals and slots must be used from the same thread.
- For multi-threaded applications, create separate port instances per thread or implement external synchronization.

---

## Compilation Flags {#compilation-flags}

The library supports various compilation flags to customize functionality:

**Feature Control Flags:**

- `MB_CLIENT_DISABLE` - Disable client functionality
- `MB_SERVER_DISABLE` - Disable server functionality
- `MB_ADDRESS_CLASS_DISABLE` - Disable Address class
- `MB_DYNAMIC_LINKING` - Enable dynamic linking (DLL/SO)

**Function-Specific Disable Flags:**

- `MBF_READ_COILS_DISABLE`
- `MBF_READ_DISCRETE_INPUTS_DISABLE`
- `MBF_READ_HOLDING_REGISTERS_DISABLE`
- `MBF_READ_INPUT_REGISTERS_DISABLE`
- `MBF_WRITE_SINGLE_COIL_DISABLE`
- `MBF_WRITE_SINGLE_REGISTER_DISABLE`
- `MBF_READ_EXCEPTION_STATUS_DISABLE`
- `MBF_DIAGNOSTICS_DISABLE`
- `MBF_GET_COMM_EVENT_COUNTER_DISABLE`
- `MBF_GET_COMM_EVENT_LOG_DISABLE`
- `MBF_WRITE_MULTIPLE_COILS_DISABLE`
- `MBF_WRITE_MULTIPLE_REGISTERS_DISABLE`
- `MBF_REPORT_SERVER_ID_DISABLE`
- `MBF_MASK_WRITE_REGISTER_DISABLE`
- `MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE`
- `MBF_READ_FIFO_QUEUE_DISABLE`

---

## Platform-Specific Notes {#platform-specific-notes}

### Windows {#windows}
- Serial ports: `COM1`, `COM2`, etc.
- TCP sockets use Winsock
- Compiler: MSVC, MinGW

### Linux/Unix {#linux-unix}
- Serial ports: `/dev/ttyS0`, `/dev/ttyUSB0`, etc.
- TCP sockets use BSD sockets
- Compiler: GCC, Clang

### Cross-Platform {#cross-platform}
- Use `Modbus::availableSerialPorts()` to enumerate ports portably
- All timeouts are in milliseconds regardless of platform
- Native handles (`Handle` type) are platform-specific

---

## See Also {#api-see-also}

- \ref architecture "Architecture Overview"
- \ref client_guide "Client Programming Guide"
- \ref server_guide "Server Programming Guide"
- Modbus specification: https://modbus.org/specs.php
