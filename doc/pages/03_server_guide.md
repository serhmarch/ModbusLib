# Server Implementation Guide {#server_guide}

## Overview {#server-overview}

The ModbusLib server implementation provides a complete framework for
implementing Modbus server (slave/device) functionality.
The library supports TCP, RTU, and ASCII protocols with a flexible architecture
that separates communication handling from application logic through the `ModbusInterface`.

**Key Features:**
* Full Modbus server implementation for TCP, RTU, and ASCII protocols
* Abstract device interface (`ModbusInterface`) for easy integration with application logic
* Support for all standard Modbus function codes (FC 01-24)
* Multiple simultaneous TCP connections with `ModbusTcpServer`
* Unit address filtering and broadcast mode support
* Non-blocking, event-driven architecture
* Comprehensive signal/callback system for monitoring
* Zero-copy operation for optimal performance

**Architecture Overview:**

The server architecture consists of three main layers:

1. **Transport Layer** (`ModbusPort` derivatives) - Handles low-level communication (TCP sockets, serial I/O)
2. **Protocol Layer** (`ModbusServerPort` derivatives) - Manages Modbus protocol framing and request/response cycles
3. **Application Layer** (`ModbusInterface` implementation) - Your device logic that processes Modbus function requests

## Server Classes {#server-classes}

### ModbusServerPort (Abstract Base) {#modbusserverport-abstract-base}

`ModbusServerPort` is the abstract base class for all server implementations. It defines the common interface and provides shared functionality like unit address filtering, broadcast mode, and event signaling.

**Key Responsibilities:**
* Device management and request delegation
* Unit address filtering via unit map
* Broadcast mode handling (unit address 0)
* Connection lifecycle management
* Signal emission for monitoring
* Context storage for user data

**Important Methods:**
```cpp
virtual Modbus::StatusCode open() = 0;        // Open server port
virtual Modbus::StatusCode close() = 0;       // Close server port
virtual Modbus::StatusCode process() = 0;     // Process requests (must be called in loop)
virtual bool isOpen() const = 0;              // Check if port is open

ModbusInterface *device() const;              // Get current device
void setDevice(ModbusInterface *device);      // Set device for request processing

void setBroadcastEnabled(bool enable);        // Enable/disable broadcast mode
void setUnitMap(const void *unitmap);         // Set unit address filter
void setContext(void *context);               // Associate user data
```

### ModbusTcpServer {#server-modbustcpserver}

`ModbusTcpServer` implements a TCP server that can handle multiple simultaneous client connections.
Each connection is managed by a separate `ModbusServerResource` instance.

**Constructor:**
```cpp
ModbusTcpServer(ModbusInterface *device);
```

**Configuration:**
```cpp
ModbusTcpServer server(&device);
server.setPort(502);              // TCP port (default: 502)
server.setTimeout(5000);          // Connection timeout in milliseconds
server.setMaxConnections(10);     // Maximum simultaneous connections
```

**Default Values:**
* port = 502 (standard Modbus TCP port)
* timeout = 3000 ms
* maxConnections = 10

**Key Features:**
* Accepts multiple client connections simultaneously
* Each connection runs independently with its own protocol handler
* Automatic connection cleanup on timeout or disconnect
* Connection event signals (new/close)
* Virtual methods for custom connection handling

**Usage Pattern:**
```cpp
MyDevice device;
ModbusTcpServer server(&device);
server.setPort(502);
server.setMaxConnections(10);

// Main server loop
while (running)
{
    server.process();  // Handle all connections
    // Add application logic here
    Modbus::msleep(1);
}
server.close();
```

### ModbusServerResource {#server-modbusserverresource}

`ModbusServerResource` wraps any `ModbusPort` (TCP, RTU, or ASCII) and makes it behave as a server port. It's used internally by `ModbusTcpServer` for each connection and can be used directly for single-connection scenarios like serial servers.

**Constructor:**
```cpp
ModbusServerResource(ModbusPort *port, ModbusInterface *device);
```

**Key Characteristics:**
* Concrete implementation of `ModbusServerPort`
* Works with any `ModbusPort` type (protocol-agnostic)
* Three-stage request processing pipeline:
  - `processInputData()` - Parse incoming request
  - `processDevice()` - Delegate to device
  - `processOutputData()` - Assemble response
* Single connection focus (use `ModbusTcpServer` for multiple TCP connections)
* Direct control over port behavior

**Usage for Serial Server:**
```cpp
MyDevice device;
ModbusRtuPort *rtuPort = new ModbusRtuPort(true);  // Blocking mode
rtuPort->setPortName("COM1");
rtuPort->setBaudRate(19200);
// Server manages the rtuPort lifecycle
ModbusServerResource server(rtuPort, &device);

while (running)
{
    server.process();
    Modbus::msleep(1);
}

server.close();
```

## Implementing ModbusInterface {#implementing-modbusinterface}

To create a Modbus server, you must implement the `ModbusInterface` class. This interface defines all Modbus function handlers that your device will support.

### Basic Implementation Pattern {#basic-implementation-pattern}

```cpp
class MyDevice : public ModbusInterface
{
private:
    uint8_t m_unit;                    // Unit address this device responds to
    std::vector<uint16_t> m_registers; // Internal register storage
    std::vector<uint8_t> m_coilBytes;  // Internal coil storage (packed bits)

public:
    MyDevice(uint8_t unit, uint16_t regCount) 
        : m_unit(unit)
    {
        m_registers.resize(regCount, 0);
        // Calculate byte count for coils (8 coils per byte)
        m_coilBytes.resize((regCount * 16 + 7) / 8, 0);
    }

    // Override Modbus functions you want to support
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                           uint16_t count, uint16_t *values) override
    {
        // Check unit address
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        // Use helper to read from internal storage
        return Modbus::readMemRegs(offset, count, values, 
                                   m_registers.data(), m_registers.size());
    }

    // Implement other functions as needed...
};
```

### ModbusInterface Method Signatures {#modbusinterface-method-signatures}

All `ModbusInterface` methods follow a consistent pattern:
* First parameter: `uint8_t unit` - Target device unit address
* Return type: `Modbus::StatusCode` - Operation result
* Default implementation returns `Status_BadIllegalFunction`

#### Function Code 01 - Read Coils {#function-code-01-read-coils}

```cpp
virtual Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, 
                                     uint16_t count, void *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting coil address (0-based)
* `count` - Number of coils to read
* `values` - Output buffer for bit-packed coil values (8 coils per byte)

#### Function Code 02 - Read Discrete Inputs {#function-code-02-read-discrete-inputs}

```cpp
virtual Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, 
                                              uint16_t count, void *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting input address (0-based)
* `count` - Number of inputs to read
* `values` - Output buffer for bit-packed input values (8 inputs per byte)

#### Function Code 03 - Read Holding Registers {#function-code-03-read-holding-registers}

```cpp
virtual Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                                uint16_t count, uint16_t *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting register address (0-based)
* `count` - Number of registers to read
* `values` - Output array of uint16_t values

#### Function Code 04 - Read Input Registers {#function-code-04-read-input-registers}

```cpp
virtual Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, 
                                              uint16_t count, uint16_t *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting register address (0-based)
* `count` - Number of registers to read
* `values` - Output array of uint16_t values

#### Function Code 05 - Write Single Coil {#function-code-05-write-single-coil}

```cpp
virtual Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, 
                                           bool value);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Coil address (0-based)
* `value` - Boolean value to write

#### Function Code 06 - Write Single Register {#function-code-06-write-single-register}

```cpp
virtual Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, 
                                               uint16_t value);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Register address (0-based)
* `value` - 16-bit value to write

#### Function Code 07 - Read Exception Status {#function-code-07-read-exception-status}

```cpp
virtual Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *status);
```
**Parameters:**
* `unit` - Device unit address
* `status` - Output byte containing exception status bits

#### Function Code 08 - Diagnostics {#function-code-08-diagnostics}

```cpp
virtual Modbus::StatusCode diagnostics(uint8_t unit, uint16_t subfunc, 
                                       uint8_t insize, const void *indata, 
                                       uint8_t *outsize, void *outdata);
```
**Parameters:**
* `unit` - Device unit address
* `subfunc` - Diagnostic sub-function code
* `insize` - Size of input data buffer (bytes)
* `indata` - Input data buffer
* `outsize` - Size of output data buffer (bytes) - set by implementation
* `outdata` - Output data buffer

#### Function Code 11 - Get Comm Event Counter {#function-code-11-get-comm-event-counter}

```cpp
virtual Modbus::StatusCode getCommEventCounter(uint8_t unit, uint16_t *status, 
                                               uint16_t *eventCount);
```
**Parameters:**
* `unit` - Device unit address
* `status` - Output status word
* `eventCount` - Output event counter value

#### Function Code 12 - Get Comm Event Log {#function-code-12-get-comm-event-log}

```cpp
virtual Modbus::StatusCode getCommEventLog(uint8_t unit, uint16_t *status, 
                                           uint16_t *eventCount, uint16_t *messageCount,
                                           uint8_t *eventBuffSize, uint8_t *eventBuff);
```
**Parameters:**
* `unit` - Device unit address
* `status` - Output status word
* `eventCount` - Output event counter
* `messageCount` - Output message counter
* `eventBuffSize` - Output event buffer size (bytes) - set by implementation
* `eventBuff` - Output event buffer (max 64 bytes)

#### Function Code 15 - Write Multiple Coils {#function-code-15-write-multiple-coils}

```cpp
virtual Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, 
                                              uint16_t count, const void *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting coil address (0-based)
* `count` - Number of coils to write
* `values` - Input buffer with bit-packed coil values (8 coils per byte)

#### Function Code 16 - Write Multiple Registers {#function-code-16-write-multiple-registers}

```cpp
virtual Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, 
                                                  uint16_t count, const uint16_t *values);
```
**Parameters:**
* `unit` - Device unit address
* `offset` - Starting register address (0-based)
* `count` - Number of registers to write
* `values` - Input array of uint16_t values

#### Function Code 17 - Report Server ID {#function-code-17-report-server-id}

```cpp
virtual Modbus::StatusCode reportServerID(uint8_t unit, uint8_t *count, 
                                          uint8_t *data);
```
**Parameters:**
* `unit` - Device unit address
* `count` - Output data size (bytes) - set by implementation
* `data` - Output buffer for server ID data (max 255 bytes)

#### Function Code 22 - Mask Write Register {#function-code-22-mask-write-register}

```cpp
virtual Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, 
                                             uint16_t andMask, uint16_t orMask);
```
**Algorithm:** `Result = (Current & andMask) | (orMask & ~andMask)`

**Parameters:**
* `unit` - Device unit address
* `offset` - Register address (0-based)
* `andMask` - AND mask value
* `orMask` - OR mask value

#### Function Code 23 - Read/Write Multiple Registers {#function-code-23-read-write-multiple-registers}

```cpp
virtual Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, 
                                                      uint16_t readOffset, uint16_t readCount, uint16_t *readValues,
                                                      uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
```
**Parameters:**
* `unit` - Device unit address
* `readOffset` - Starting read register address (0-based)
* `readCount` - Number of registers to read
* `readValues` - Output array for read values
* `writeOffset` - Starting write register address (0-based)
* `writeCount` - Number of registers to write
* `writeValues` - Input array of values to write

**Note:** Write operation should be performed before read operation for atomic behavior.

#### Function Code 24 - Read FIFO Queue {#function-code-24-read-fifo-queue}

```cpp
virtual Modbus::StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, 
                                         uint16_t *count, uint16_t *values);
```
**Parameters:**
* `unit` - Device unit address
* `fifoadr` - FIFO address
* `count` - Output count of values read - set by implementation (max 31)
* `values` - Output array for FIFO values

## Complete Server Examples {#complete-server-examples}

### TCP Server Example {#server-tcp-example}

Complete TCP server with holding registers and coils support:

```cpp
#include <iostream>
#include <vector>
#include <ModbusTcpServer.h>
#include <Modbus.h>

// Device implementation with register and coil storage
class TcpDevice : public ModbusInterface
{
private:
    uint8_t m_unit;
    std::vector<uint16_t> m_holdingRegs;
    std::vector<uint16_t> m_inputRegs;
    std::vector<uint8_t> m_coilBytes;
    std::vector<uint8_t> m_inputBytes;

public:
    TcpDevice(uint8_t unit) : m_unit(unit)
    {
        // Initialize 1000 registers and 1000 coils
        m_holdingRegs.resize(1000, 0);
        m_inputRegs.resize(1000, 0);
        m_coilBytes.resize(125, 0);     // 1000 bits = 125 bytes
        m_inputBytes.resize(125, 0);
    }

    Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, 
                                 uint16_t count, void *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemBits(offset, count, values, 
                                   m_coilBytes.data(), m_coilBytes.size() * 8);
    }

    Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, 
                                          uint16_t count, void *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemBits(offset, count, values, 
                                   m_inputBytes.data(), m_inputBytes.size() * 8);
    }

    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                            uint16_t count, uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemRegs(offset, count, values, 
                                   m_holdingRegs.data(), m_holdingRegs.size());
    }

    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, 
                                          uint16_t count, uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemRegs(offset, count, values, 
                                   m_inputRegs.data(), m_inputRegs.size());
    }

    Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, 
                                       bool value) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemBits(offset, 1, &value, 
                                    m_coilBytes.data(), m_coilBytes.size() * 8);
    }

    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, 
                                           uint16_t value) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemRegs(offset, 1, &value, 
                                    m_holdingRegs.data(), m_holdingRegs.size());
    }

    Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, 
                                          uint16_t count, const void *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemBits(offset, count, values, 
                                    m_coilBytes.data(), m_coilBytes.size() * 8);
    }

    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, 
                                              uint16_t count, const uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemRegs(offset, count, values, 
                                    m_holdingRegs.data(), m_holdingRegs.size());
    }

    Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, 
                                         uint16_t andMask, uint16_t orMask) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        // Read current value
        uint16_t current;
        Modbus::StatusCode status = Modbus::readMemRegs(offset, 1, &current, 
                                                        m_holdingRegs.data(), m_holdingRegs.size());
        if (Modbus::StatusIsBad(status))
            return status;
        
        // Apply mask operation: (current & andMask) | (orMask & ~andMask)
        uint16_t result = (current & andMask) | (orMask & ~andMask);
        
        return Modbus::writeMemRegs(offset, 1, &result, 
                                    m_holdingRegs.data(), m_holdingRegs.size());
    }

    Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, 
                                                  uint16_t readOffset, uint16_t readCount, uint16_t *readValues,
                                                  uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        // Perform write first, then read (atomic operation)
        Modbus::StatusCode status = Modbus::writeMemRegs(writeOffset, writeCount, writeValues,
                                                         m_holdingRegs.data(), m_holdingRegs.size());
        if (Modbus::StatusIsBad(status))
            return status;
        
        return Modbus::readMemRegs(readOffset, readCount, readValues,
                                   m_holdingRegs.data(), m_holdingRegs.size());
    }

    // Increment first register (for demo purposes)
    void increment() { m_holdingRegs[0]++; }
};

// Callback functions for monitoring
void onTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Tx: " << Modbus::bytesToString(buff, size) << std::endl;
}

void onRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Rx: " << Modbus::bytesToString(buff, size) << std::endl;
}

void onNewConnection(const Modbus::Char *source)
{
    std::cout << "New connection: " << source << std::endl;
}

void onCloseConnection(const Modbus::Char *source)
{
    std::cout << "Close connection: " << source << std::endl;
}

void onError(const Modbus::Char *source, Modbus::StatusCode code, const Modbus::Char *text)
{
    std::cerr << "Error from " << source << ": " << text << std::endl;
}

int main()
{
    // Create device for unit address 1
    TcpDevice device(1);
    
    // Create TCP server
    ModbusTcpServer server(&device);
    server.setPort(502);
    server.setTimeout(5000);
    server.setMaxConnections(10);
    
    // Connect monitoring callbacks
    server.connect(&ModbusTcpServer::signalTx, onTx);
    server.connect(&ModbusTcpServer::signalRx, onRx);
    server.connect(&ModbusTcpServer::signalNewConnection, onNewConnection);
    server.connect(&ModbusTcpServer::signalCloseConnection, onCloseConnection);
    server.connect(&ModbusTcpServer::signalError, onError);
    
    // Open server
    std::cout << "Starting Modbus TCP server on port 502..." << std::endl;
    // Main server loop
    bool running = true;
    Modbus::Timer lastIncrement = Modbus::timer();
    
    while (running)
    {
        // Process all connections
        server.process();
        
        // Increment counter every second (demo)
        Modbus::Timer now = Modbus::timer();
        if ((now - lastIncrement) >= 1000)
        {
            device.increment();
            lastIncrement = now;
        }
        
        // Small delay to prevent CPU spinning
        Modbus::msleep(1);
    }
    
    // Clean shutdown
    server.close();
    std::cout << "Server stopped" << std::endl;
    
    return 0;
}
```

### RTU Serial Server Example {#rtu-serial-server-example}

Complete RTU server implementation for serial communication:

```cpp
#include <iostream>
#include <vector>
#include <ModbusServerResource.h>
#include <ModbusRtuPort.h>
#include <Modbus.h>

// Device implementation for RTU server
class RtuDevice : public ModbusInterface
{
private:
    uint8_t m_unit;
    std::vector<uint16_t> m_registers;

public:
    RtuDevice(uint8_t unit, uint16_t regCount) : m_unit(unit)
    {
        m_registers.resize(regCount, 0);
    }

    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                            uint16_t count, uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemRegs(offset, count, values, 
                                   m_registers.data(), m_registers.size());
    }

    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, 
                                          uint16_t count, uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::readMemRegs(offset, count, values, 
                                   m_registers.data(), m_registers.size());
    }

    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, 
                                           uint16_t value) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemRegs(offset, 1, &value, 
                                    m_registers.data(), m_registers.size());
    }

    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, 
                                              uint16_t count, const uint16_t *values) override
    {
        if (unit != m_unit)
            return Modbus::Status_BadGatewayPathUnavailable;
        
        return Modbus::writeMemRegs(offset, count, values, 
                                    m_registers.data(), m_registers.size());
    }

    void setRegister(uint16_t offset, uint16_t value)
    {
        if (offset < m_registers.size())
            m_registers[offset] = value;
    }
};

// Monitoring callbacks
void onTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Tx: " << Modbus::bytesToString(buff, size) << std::endl;
}

void onRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << source << " Rx: " << Modbus::bytesToString(buff, size) << std::endl;
}

int main()
{
    // Create device for unit address 1 with 100 registers
    RtuDevice device(1, 100);
    
    // Create and configure RTU port
    ModbusRtuPort rtuPort(false);  // Non-blocking mode
    
#ifdef _WIN32
    rtuPort.setPortName("COM1");
#else
    rtuPort.setPortName("/dev/ttyS0");
#endif
    
    rtuPort.setBaudRate(19200);
    rtuPort.setDataBits(8);
    rtuPort.setParity(Modbus::NoParity);
    rtuPort.setStopBits(Modbus::OneStop);
    rtuPort.setTimeoutFirstByte(3000);
    rtuPort.setTimeoutInterByte(10);
    
    // Create server resource
    ModbusServerResource server(&rtuPort, &device);
    
    // Connect monitoring callbacks
    server.connect(&ModbusServerResource::signalTx, onTx);
    server.connect(&ModbusServerResource::signalRx, onRx);
    
    std::cout << "Starting Modbus RTU server..." << std::endl;
    // Main server loop
    bool running = true;
    while (running)
    {
        // Process requests
        server.process();
        
        // Add application logic here
        
        Modbus::msleep(1);
    }
    
    // Clean shutdown
    server.close();
    std::cout << "Server stopped" << std::endl;
    
    return 0;
}
```

### Multi-Device Server Example {#multi-device-server-example}

Server supporting multiple unit addresses:

```cpp
#include <iostream>
#include <map>
#include <vector>
#include <ModbusTcpServer.h>
#include <Modbus.h>

// Individual device with its own memory
class SubDevice
{
public:
    std::vector<uint16_t> holdingRegs;
    std::vector<uint16_t> inputRegs;
    
    SubDevice(uint16_t regCount)
    {
        holdingRegs.resize(regCount, 0);
        inputRegs.resize(regCount, 0);
    }
};

// Multi-device dispatcher
class MultiDevice : public ModbusInterface
{
private:
    std::map<uint8_t, SubDevice*> m_devices;

public:
    void addDevice(uint8_t unit, uint16_t regCount)
    {
        m_devices[unit] = new SubDevice(regCount);
    }

    ~MultiDevice()
    {
        for (auto &pair : m_devices)
            delete pair.second;
    }

    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                            uint16_t count, uint16_t *values) override
    {
        auto it = m_devices.find(unit);
        if (it == m_devices.end())
            return Modbus::Status_BadGatewayPathUnavailable;
        
        SubDevice *dev = it->second;
        return Modbus::readMemRegs(offset, count, values, 
                                   dev->holdingRegs.data(), dev->holdingRegs.size());
    }

    Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, 
                                          uint16_t count, uint16_t *values) override
    {
        auto it = m_devices.find(unit);
        if (it == m_devices.end())
            return Modbus::Status_BadGatewayPathUnavailable;
        
        SubDevice *dev = it->second;
        return Modbus::readMemRegs(offset, count, values, 
                                   dev->inputRegs.data(), dev->inputRegs.size());
    }

    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, 
                                           uint16_t value) override
    {
        auto it = m_devices.find(unit);
        if (it == m_devices.end())
            return Modbus::Status_BadGatewayPathUnavailable;
        
        SubDevice *dev = it->second;
        return Modbus::writeMemRegs(offset, 1, &value, 
                                    dev->holdingRegs.data(), dev->holdingRegs.size());
    }

    Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, 
                                              uint16_t count, const uint16_t *values) override
    {
        auto it = m_devices.find(unit);
        if (it == m_devices.end())
            return Modbus::Status_BadGatewayPathUnavailable;
        
        SubDevice *dev = it->second;
        return Modbus::writeMemRegs(offset, count, values, 
                                    dev->holdingRegs.data(), dev->holdingRegs.size());
    }
};

int main()
{
    // Create multi-device
    MultiDevice multiDevice;
    multiDevice.addDevice(1, 100);  // Device at unit 1 with 100 registers
    multiDevice.addDevice(2, 200);  // Device at unit 2 with 200 registers
    multiDevice.addDevice(3, 50);   // Device at unit 3 with 50 registers
    
    // Create TCP server
    ModbusTcpServer server(&multiDevice);
    server.setPort(502);
    server.setMaxConnections(5);
    
    std::cout << "Server running - supports units 1, 2, 3" << std::endl;
        
    bool running = true;
    while (running)
    {
        server.process();
        Modbus::msleep(1);
    }
    
    return 0;
}
```

## Error Handling Patterns {#server-error-handling}

### Returning Status Codes {#returning-status-codes}

All `ModbusInterface` methods must return appropriate status codes:

```cpp
Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                        uint16_t count, uint16_t *values) override
{
    // Check unit address
    if (unit != m_unit)
        return Modbus::Status_BadGatewayPathUnavailable;
    
    // Check address range
    if (offset + count > m_registers.size())
        return Modbus::Status_BadIllegalDataAddress;
    
    // Check count limits
    if (count == 0 || count > MB_MAX_REGISTERS)
        return Modbus::Status_BadIllegalDataValue;
    
    // Perform operation
    for (uint16_t i = 0; i < count; i++)
        values[i] = m_registers[offset + i];
    
    return Modbus::Status_Good;
}
```

### Standard Modbus Exception Codes {#standard-modbus-exception-codes}

The library provides standard Modbus exception codes:

* `Status_BadIllegalFunction` (0x01) - Function not supported
* `Status_BadIllegalDataAddress` (0x02) - Invalid register/coil address
* `Status_BadIllegalDataValue` (0x03) - Invalid data value or count
* `Status_BadServerDeviceFailure` (0x04) - Device failure
* `Status_BadAcknowledge` (0x05) - Long operation in progress
* `Status_BadServerDeviceBusy` (0x06) - Device busy
* `Status_BadNegativeAcknowledge` (0x07) - Cannot perform operation
* `Status_BadMemoryParityError` (0x08) - Memory parity error
* `Status_BadGatewayPathUnavailable` (0x0A) - Gateway path unavailable (wrong unit)
* `Status_BadGatewayTargetDeviceFailedToRespond` (0x0B) - Target device failed to respond

### Using Helper Functions {#using-helper-functions}

ModbusLib provides helper functions for memory operations:

```cpp
// Read registers with automatic bounds checking
Modbus::StatusCode readMemRegs(uint32_t offset, uint32_t count, 
                               void *values, const void *memBuff, 
                               uint32_t memRegCount);

// Write registers with automatic bounds checking
Modbus::StatusCode writeMemRegs(uint32_t offset, uint32_t count, 
                                const void *values, void *memBuff, 
                                uint32_t memRegCount);

// Read bits with automatic bounds checking
Modbus::StatusCode readMemBits(uint32_t offset, uint32_t count, 
                               void *values, const void *memBuff, 
                               uint32_t memBitCount);

// Write bits with automatic bounds checking
Modbus::StatusCode writeMemBits(uint32_t offset, uint32_t count, 
                                const void *values, void *memBuff, 
                                uint32_t memBitCount);
```

These helpers automatically return appropriate error codes for out-of-range addresses.

## Unit Address Filtering {#unit-address-filtering}

### Unit Map Mechanism {#unit-map-mechanism}

The unit map is a 32-byte bitmap (256 bits) for filtering unit addresses:

```cpp
// Create unit map (32 bytes for 256 unit addresses)
uint8_t unitMap[32] = {0};

// Enable specific units
MB_UNITMAP_SET_BIT(unitMap, 1, 1);   // Enable unit 1
MB_UNITMAP_SET_BIT(unitMap, 2, 1);   // Enable unit 2
MB_UNITMAP_SET_BIT(unitMap, 10, 1);  // Enable unit 10

// Apply to server
server.setUnitMap(unitMap);

// Check if unit is enabled
bool isEnabled = MB_UNITMAP_GET_BIT(unitMap, 1);
```

**Behavior:**
* If no unit map is set (nullptr), all units are accepted
* If unit map is set, only enabled units are processed
* Disabled units receive no response (as per Modbus specification)

### Broadcast Mode {#broadcast-mode}

Broadcast mode allows unit address 0 to trigger all devices:

```cpp
server.setBroadcastEnabled(true);   // Enable broadcast (default)
server.setBroadcastEnabled(false);  // Disable broadcast
```

When broadcast is enabled:
* Requests to unit 0 are processed
* No response is sent (per Modbus specification)
* Only write operations should be broadcast

## Signal and Callback System {#server-signal-callback}

### Available Signals {#available-signals}

All server classes emit these signals:

```cpp
// Port lifecycle
void signalOpened(const Modbus::Char *source);
void signalClosed(const Modbus::Char *source);

// Communication monitoring
void signalTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);
void signalRx(const Modbus::Char *source, const uint8_t* buff, uint16_t size);

// Error reporting
void signalError(const Modbus::Char *source, Modbus::StatusCode status, 
                 const Modbus::Char *text);

// TCP-specific (ModbusTcpServer only)
void signalNewConnection(const Modbus::Char *source);
void signalCloseConnection(const Modbus::Char *source);
```

### Connecting Callbacks {#connecting-callbacks}

```cpp
// Define callback functions
void onTx(const Modbus::Char *source, const uint8_t* buff, uint16_t size)
{
    std::cout << "Tx from " << source << ": " 
              << Modbus::bytesToString(buff, size) << std::endl;
}

void onError(const Modbus::Char *source, Modbus::StatusCode status, 
             const Modbus::Char *text)
{
    std::cerr << "Error from " << source << ": " << text << std::endl;
}

// Connect callbacks
server.connect(&ModbusTcpServer::signalTx, onTx);
server.connect(&ModbusTcpServer::signalError, onError);
```

## Best Practices {#server-best-practices}

### 1. Non-Blocking Operation {#1-non-blocking-operation}

Always use non-blocking mode for servers to handle multiple operations:

```cpp
ModbusRtuPort rtuPort(false);  // false = non-blocking
```

In non-blocking mode, call `process()` repeatedly in your main loop.

### 2. Timeout Configuration {#2-timeout-configuration}

Set appropriate timeouts based on your network:

```cpp
// TCP: Longer timeouts for WAN, shorter for LAN
server.setTimeout(5000);  // 5 seconds for WAN
server.setTimeout(1000);  // 1 second for LAN

// Serial: Based on baud rate and expected response time
rtuPort.setTimeoutFirstByte(1000);
rtuPort.setTimeoutInterByte(10);
```

### 3. Memory Management {#3-memory-management}

Pre-allocate memory in your device implementation:

```cpp
class MyDevice : public ModbusInterface
{
private:
    std::vector<uint16_t> m_registers;

public:
    MyDevice(uint16_t regCount)
    {
        m_registers.resize(regCount, 0);  // Pre-allocate
    }
};
```

### 4. Thread Safety {#4-thread-safety}

ModbusLib is not inherently thread-safe. If using multiple threads:

```cpp
#include <mutex>

class ThreadSafeDevice : public ModbusInterface
{
private:
    std::mutex m_mutex;
    std::vector<uint16_t> m_registers;

public:
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                            uint16_t count, uint16_t *values) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Perform operation with mutex protection
        return Modbus::readMemRegs(offset, count, values, 
                                   m_registers.data(), m_registers.size());
    }
};
```

### 5. Error Logging {#5-error-logging}

Implement comprehensive error logging:

```cpp
void onError(const Modbus::Char *source, Modbus::StatusCode status, 
             const Modbus::Char *text)
{
    // Log to file or monitoring system
    std::cerr << "[" << Modbus::getTimestamp() << "] "
              << "Error from " << source 
              << " (0x" << std::hex << status << "): " 
              << text << std::endl;
}
```

### 6. Resource Cleanup {#6-resource-cleanup}

Always close servers properly:

```cpp
// Use RAII or explicit cleanup
void runServer()
{
    ModbusTcpServer server(&device);
    server.open();
    
    try {
        while (running)
            server.process();
    }
    catch (...) {
        server.close();
        throw;
    }
    
    server.close();
}
```

### 7. Validate All Inputs {#7-validate-all-inputs}

Always validate parameters in your `ModbusInterface` implementation:

```cpp
Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, 
                                          uint16_t count, const uint16_t *values) override
{
    // Check unit
    if (unit != m_unit)
        return Modbus::Status_BadGatewayPathUnavailable;
    
    // Check count
    if (count == 0 || count > MB_MAX_REGISTERS)
        return Modbus::Status_BadIllegalDataValue;
    
    // Check address range
    if (offset + count > m_registers.size())
        return Modbus::Status_BadIllegalDataAddress;
    
    // Validate values if needed
    for (uint16_t i = 0; i < count; i++)
    {
        if (values[i] > MAX_ALLOWED_VALUE)
            return Modbus::Status_BadIllegalDataValue;
    }
    
    // Perform operation
    return Modbus::writeMemRegs(offset, count, values, 
                                m_registers.data(), m_registers.size());
}
```

### 8. Performance Optimization {#8-performance-optimization}

For high-performance servers:

```cpp
// Use memcpy for bulk operations
Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                        uint16_t count, uint16_t *values) override
{
    // Validation omitted for brevity
    
    // Fast memory copy
    memcpy(values, &m_registers[offset], count * sizeof(uint16_t));
    return Modbus::Status_Good;
}

// Minimize allocations in hot path
// Pre-allocate buffers, use object pools, etc.
```

### 9. Monitoring and Diagnostics {#9-monitoring-and-diagnostics}

Implement comprehensive monitoring:

```cpp
class MonitoredDevice : public ModbusInterface
{
private:
    uint32_t m_readCount;
    uint32_t m_writeCount;
    uint32_t m_errorCount;

public:
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                            uint16_t count, uint16_t *values) override
    {
        m_readCount++;
        Modbus::StatusCode status = /* perform read */;
        
        if (Modbus::StatusIsBad(status))
            m_errorCount++;
        
        return status;
    }
    
    void printStatistics()
    {
        std::cout << "Reads: " << m_readCount 
                  << ", Writes: " << m_writeCount
                  << ", Errors: " << m_errorCount << std::endl;
    }
};
```

### 10. Configuration Management {#10-configuration-management}

Use settings structures for easy configuration:

```cpp
struct ServerConfig
{
    uint16_t port;
    uint32_t timeout;
    uint32_t maxConnections;
    std::string logFile;
};

void configureServer(ModbusTcpServer &server, const ServerConfig &config)
{
    server.setPort(config.port);
    server.setTimeout(config.timeout);
    server.setMaxConnections(config.maxConnections);
    // Setup logging, etc.
}
```

## See Also {#server-see-also}

* \ref client_guide "Client Implementation Guide" - For client-side implementation
* \ref architecture "Architecture Overview" - For overall library architecture
* `ModbusInterface` - Complete API reference
* `ModbusTcpServer` - TCP server class reference
* `ModbusServerResource` - Server resource class reference
* `ModbusServerPort` - Base server port class reference
