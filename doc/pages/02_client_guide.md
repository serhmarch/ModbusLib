# Client Implementation Guide {#client_guide}

## Overview {#client-overview}

The ModbusLib client implementation provides comprehensive support for Modbus communication with devices. The library offers both synchronous (blocking) and asynchronous (non-blocking) client interfaces through multiple abstraction levels.

## Client Classes {#client-guide-classes}

### ModbusClientPort {#client-modbusclientport}

`ModbusClientPort` is the core client implementation that directly implements the `ModbusInterface`. It manages the complete request-response cycle, including error handling, timeouts, and state management.

**Key Features:**
* Implements all Modbus function codes (FC 01-24)
* Blocking and non-blocking operation modes
* Automatic connection management
* Comprehensive error handling
* Signal/slot event system (Qt mode)
* Resource sharing between multiple clients

**Constructor:**
```cpp
ModbusClientPort(ModbusPort *port);
```

**Parameters:**
* `port` - A `ModbusPort` instance (TCP, RTU, or ASCII)

**Example - Basic Usage:**
```cpp
#include "ModbusTcpPort.h"
#include "ModbusClientPort.h"

// Create TCP port
ModbusTcpPort tcpPort;
tcpPort.setHost("192.168.1.100");
tcpPort.setPort(502);

// Create client port
ModbusClientPort clientPort(&tcpPort);

// Read holding registers (FC 03)
uint16_t registers[10];
Modbus::StatusCode status = clientPort.readHoldingRegisters(1, 0, 10, registers);
if (Modbus::StatusIsGood(status))
{
    // Process register values
}
```

### ModbusClient {#client-modbusclient}

`ModbusClient` is a convenience wrapper around `ModbusClientPort` that eliminates the need to specify the unit identifier for every operation. It's especially useful when managing multiple Modbus devices on the same network.

**Key Features:**
* Stores unit identifier internally
* Simplified method signatures
* Support for multiple clients on single port
* Automatic port resource sharing

**Constructor:**
```cpp
ModbusClient(uint8_t unit, ModbusClientPort *port);
```

**Parameters:**
* `unit` - Modbus device unit/slave address
* `port` - A `ModbusClientPort` instance

**Example - Multiple Devices:**
```cpp
#include "ModbusClient.h"
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"

ModbusTcpPort tcpPort;
tcpPort.setHost("192.168.1.100");
ModbusClientPort port(&tcpPort);

// Create clients for different units
ModbusClient client1(1, &port);
ModbusClient client2(2, &port);
ModbusClient client3(3, &port);

// Access devices without specifying unit each time
uint16_t data1[10];
uint16_t data2[10];
uint16_t data3[10];

client1.readHoldingRegisters(0, 10, data1);
client2.readHoldingRegisters(0, 10, data2);
client3.readHoldingRegisters(0, 10, data3);
```

## Port Types {#port-types}

### TCP Client Port {#tcp-client-port}

`ModbusTcpPort` - TCP protocol implementation for network communication.

**Constructor:**
```cpp
ModbusTcpPort();
```

**Settings:**
```cpp
ModbusTcpPort port;
port.setHost("192.168.1.100");    // Device IP or hostname
port.setPort(502);                 // Modbus TCP port (default: 502)
port.setTimeout(3000);             // Timeout in milliseconds
```

**Default Values:**
* host = "localhost"
* port = 502
* timeout = 3000

### RTU Serial Port {#rtu-serial-port}

`ModbusRtuPort` - RTU protocol implementation for serial communication.

**Constructor:**
```cpp
ModbusRtuPort();
```

**Settings:**
```cpp
ModbusRtuPort port;
port.setPortName("COM1");          // Serial port
port.setBaudRate(9600);            // Baud rate
port.setDataBits(8);               // Data bits (5-8)
port.setParity(Modbus::NoParity);  // Parity
port.setStopBits(Modbus::OneStop); // Stop bits
port.setTimeoutFirstByte(3000);    // First byte timeout (ms)
port.setTimeoutInterByte(5);       // Inter-byte timeout (ms)
```

**Default Values:**
* portName = "COM1" (Windows) or "/dev/ttyS0" (Unix)
* baudRate = 9600
* dataBits = 8
* parity = NoParity
* stopBits = OneStop
* timeoutFirstByte = 3000
* timeoutInterByte = 5

### ASCII Serial Port {#ascii-serial-port}

`ModbusAscPort` - ASCII protocol implementation for serial communication.

Uses same configuration as RTU but with ASCII frame encoding.

**Constructor:**
```cpp
ModbusAscPort();
```

## Operation Modes {#operation-modes}

### Blocking Mode {#client-blocking-mode}

In blocking mode, method calls wait until the operation completes and returns the result directly.

**Characteristics:**
* Simpler to understand and debug
* Entire thread blocks during I/O
* Returns result or error status immediately
* Suitable for synchronous applications

**Example:**
```cpp
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"

ModbusTcpPort tcp;
tcp.setHost("192.168.1.100");
ModbusClientPort port(&tcp);

// Method blocks until response received
uint16_t registers[10];
Modbus::StatusCode status = port.readHoldingRegisters(1, 0, 10, registers);
if (Modbus::StatusIsGood(status))
{
    // Process registers
}
else
{
    // Handle error
    const Modbus::Char *errorText = port.lastErrorText();
}
```

### Non-Blocking Mode {#client-non-blocking-mode}

In non-blocking mode, method calls return immediately. If the operation is incomplete, they return `Status_Processing`. The caller must retry the operation.

**Characteristics:**
* Application maintains control loop
* Returns `Status_Processing` if operation incomplete
* Allows concurrent operations in polling loop
* Higher CPU usage due to polling
* Requires polling logic in application

**Example:**
```cpp
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"

ModbusTcpPort tcp;
tcp.setHost("192.168.1.100");
ModbusClientPort port(&tcp);

uint16_t registers[10];

while (true)
{
    Modbus::StatusCode status = port.readHoldingRegisters(1, 0, 10, registers);
    
    if (Modbus::StatusIsGood(status))
    {
        // Success
        break;
    }
    else if (Modbus::StatusIsProcessing(status))
    {
        // Operation not complete yet, do other work
        // ...
        Modbus::msleep(1);
    }
    else
    {
        // Error occurred
        break;
    }
}
```

## Modbus Functions {#modbus-functions}

### Read Functions {#read-functions}

#### readCoils (Function Code 01) {#readcoils-function-code-01}

Reads discrete outputs (coils, 0x bits).

**Signature:**
```cpp
Modbus::StatusCode readCoils(uint8_t unit, uint16_t offset, uint16_t count, void *values);
```

**Parameters:**
* `unit` - Remote device unit/slave address
* `offset` - Starting coil offset (0-based)
* `count` - Number of coils to read
* `values` - Output buffer (packed 8 bits per byte)

**Returns:**
* `StatusCode` - Operation status

**Example:**
```cpp
// Read 16 coils starting at offset 0
uint8_t coilData[2]; // 16 coils = 2 bytes
Modbus::StatusCode status = client.readCoils(0, 16, coilData);
if (Modbus::StatusIsGood(status))
{
    for (int i = 0; i < 16; i++)
    {
        uint8_t byteIdx = i / 8;
        uint8_t bitIdx = i % 8;
        bool bitVal = (coilData[byteIdx] >> bitIdx) & 1;
        // Process coil value
    }
}
```

#### readDiscreteInputs (Function Code 02) {#readdiscreteinputs-function-code-02}

Reads digital inputs (1x bits). Read-only version of coils.

**Signature:**
```cpp
Modbus::StatusCode readDiscreteInputs(uint8_t unit, uint16_t offset, uint16_t count, void *values);
```

#### readHoldingRegisters (Function Code 03) {#readholdingregisters-function-code-03}

Reads holding (output) 16-bit registers (4x regs).

**Signature:**
```cpp
Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
```

**Parameters:**
* `unit` - Remote device unit/slave address
* `offset` - Starting register offset (0-based)
* `count` - Number of registers to read
* `values` - Output array of uint16_t

**Returns:**
* `StatusCode` - Operation status

**Example:**
```cpp
// Read 10 registers
uint16_t registers[10];
Modbus::StatusCode status = client.readHoldingRegisters(0, 10, registers);
if (Modbus::StatusIsGood(status))
{
    for (int i = 0; i < 10; i++)
    {
        // Process register value
        uint16_t value = registers[i];
    }
}
```

#### readInputRegisters (Function Code 04) {#readinputregisters-function-code-04}

Reads input 16-bit registers (3x regs). Read-only version of holding registers.

**Signature:**
```cpp
Modbus::StatusCode readInputRegisters(uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values);
```

### Write Functions {#write-functions}

#### writeSingleCoil (Function Code 05) {#writesinglecoil-function-code-05}

Writes single coil (discrete output).

**Signature:**
```cpp
Modbus::StatusCode writeSingleCoil(uint8_t unit, uint16_t offset, bool value);
```

**Parameters:**
* `unit` - Remote device unit/slave address
* `offset` - Coil offset (0-based)
* `value` - Boolean value to write

**Returns:**
* `StatusCode` - Operation status

**Example:**
```cpp
Modbus::StatusCode status = client.writeSingleCoil(0, true);
if (Modbus::StatusIsGood(status))
{
    // Coil written successfully
}
```

#### writeSingleRegister (Function Code 06) {#writesingleregister-function-code-06}

Writes single 16-bit register.

**Signature:**
```cpp
Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset, uint16_t value);
```

**Example:**
```cpp
Modbus::StatusCode status = client.writeSingleRegister(0, 1234);
if (Modbus::StatusIsGood(status))
{
    // Register written successfully
}
```

#### writeMultipleCoils (Function Code 15) {#writemultiplecoils-function-code-15}

Writes multiple coils.

**Signature:**
```cpp
Modbus::StatusCode writeMultipleCoils(uint8_t unit, uint16_t offset, uint16_t count, const void *values);
```

**Parameters:**
* `unit` - Remote device unit/slave address
* `offset` - Starting coil offset (0-based)
* `count` - Number of coils to write
* `values` - Bit array (packed 8 bits per byte)

**Returns:**
* `StatusCode` - Operation status

**Example:**
```cpp
// Write 8 coils: on, off, on, off, on, off, on, off
uint8_t coilValues = 0b01010101;
Modbus::StatusCode status = client.writeMultipleCoils(0, 8, &coilValues);
```

#### writeMultipleRegisters (Function Code 16) {#writemultipleregisters-function-code-16}

Writes multiple 16-bit registers.

**Signature:**
```cpp
Modbus::StatusCode writeMultipleRegisters(uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values);
```

**Example:**
```cpp
// Write 3 registers with values 1000, 2000, 3000
uint16_t registerValues[] = {1000, 2000, 3000};
Modbus::StatusCode status = client.writeMultipleRegisters(0, 3, registerValues);
```

### Advanced Functions {#advanced-functions}

#### maskWriteRegister (Function Code 22) {#maskwriteregister-function-code-22}

Performs bitwise AND/OR mask operation on single register.

**Signature:**
```cpp
Modbus::StatusCode maskWriteRegister(uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask);
```

**Logic:** `(current_value & and_mask) | (or_mask & ~and_mask)`

**Example:**
```cpp
// Set bits 4-7, clear bits 0-3
uint16_t andMask = 0x00F0;  // Keep high nibble
uint16_t orMask = 0x00F0;   // Set high nibble
Modbus::StatusCode status = client.maskWriteRegister(0, andMask, orMask);
```

#### readWriteMultipleRegisters (Function Code 23) {#readwritemultipleregisters-function-code-23}

Atomically reads and writes registers in single operation.

**Signature:**
```cpp
Modbus::StatusCode readWriteMultipleRegisters(uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues);
```

**Example:**
```cpp
uint16_t writeData[] = {100, 200};
uint16_t readData[5];
Modbus::StatusCode status = client.readWriteMultipleRegisters(
    0, 5, readData,     // Read 5 registers from offset 0
    10, 2, writeData    // Write 2 registers to offset 10
);
```

#### readExceptionStatus (Function Code 07) {#readexceptionstatus-function-code-07}

Reads exception status (diagnostic function).

**Signature:**
```cpp
Modbus::StatusCode readExceptionStatus(uint8_t unit, uint8_t *value);
```

#### diagnostics (Function Code 08) {#diagnostics-function-code-08}

Sends diagnostic command to device.

**Signature:**
```cpp
Modbus::StatusCode diagnostics(uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata);
```

#### readFIFOQueue (Function Code 24) {#readfifoqueue-function-code-24}

Reads contents of FIFO queue.

**Signature:**
```cpp
Modbus::StatusCode readFIFOQueue(uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values);
```

#### reportServerID (Function Code 17) {#reportserverid-function-code-17}

Gets server identification information.

**Signature:**
```cpp
Modbus::StatusCode reportServerID(uint8_t unit, uint8_t *count, uint8_t *data);
```

## Error Handling {#client-error-handling}

### Status Checking {#status-checking}

```cpp
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"

ModbusTcpPort tcp;
ModbusClientPort port(&tcp);

uint16_t registers[10];
Modbus::StatusCode status = port.readHoldingRegisters(1, 0, 10, registers);

if (Modbus::StatusIsGood(status))
{
    // Success
}
else if (Modbus::StatusIsProcessing(status))
{
    // Still processing (non-blocking mode)
}
else
{
    // Error occurred
    Modbus::StatusCode errorStatus = port.lastErrorStatus();
    const Modbus::Char *errorText = port.lastErrorText();
    // Handle error
}
```

### Status Codes {#client-status-codes}

Status codes indicate the status of last operation:

* `Status_Good` - Successful operation
* `Status_Processing` - Operation in progress (non-blocking)
* `Status_Bad*` - Various error codes
* Standard Modbus exceptions (0x01-0x0B)
* Protocol-specific errors

## Signal and Callback System {#client-signal-callback}

### Connecting Callbacks (Qt Mode) {#connecting-callbacks-qt-mode}

```cpp
void onTx(const Modbus::Char *source, const uint8_t* buffer, uint16_t size)
{
    // Handle transmit event
}

void onRx(const Modbus::Char *source, const uint8_t* buffer, uint16_t size)
{
    // Handle receive event
}

void onError(const Modbus::Char *source, Modbus::StatusCode code, const Modbus::Char *text)
{
    // Handle error
}

// Connect signals (Qt mode)
port->connect(&ModbusClientPort::signalTx, onTx);
port->connect(&ModbusClientPort::signalRx, onRx);
port->connect(&ModbusClientPort::signalError, onError);
```

## Complete Examples {#complete-examples}

### TCP Client Example {#client-tcp-example}

```cpp
#include <iostream>
#include "ModbusClient.h"
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"

int main()
{
    // Create TCP port
    ModbusTcpPort *tcp = new ModbusTcpPort();
    tcp->setHost("192.168.1.100");
    tcp->setPort(502);
    tcp->setTimeout(3000);
    
    // Create port and client
    ModbusClientPort port(tcp);
    ModbusClient client(1, &port);
    
    // Read 10 holding registers
    uint16_t data[10];
    Modbus::StatusCode status = client.readHoldingRegisters(0, 10, data);
    
    if (Modbus::StatusIsGood(status))
    {
        std::cout << "Read success" << std::endl;
        for (int i = 0; i < 10; i++)
        {
            std::cout << "Register[" << i << "] = " << data[i] << std::endl;
        }
        
        // Write single register
        status = client.writeSingleRegister(100, 5678);
        if (Modbus::StatusIsGood(status))
        {
            std::cout << "Write successful" << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: " << port.lastErrorText() << std::endl;
    }
    
    return 0;
}
```

### RTU Client Example {#client-rtu-example}

```cpp
#include <iostream>
#include "ModbusClient.h"
#include "ModbusClientPort.h"
#include "ModbusRtuPort.h"

int main()
{
    // Create RTU port
    ModbusRtuPort *rtu = new ModbusRtuPort();
    rtu->setPortName("COM1");
    rtu->setBaudRate(19200);
    rtu->setDataBits(8);
    rtu->setParity(Modbus::NoParity);
    rtu->setStopBits(Modbus::OneStop);
    rtu->open();
    // Create port and client
    ModbusClientPort port(rtu);
    ModbusClient client(1, &port);
    
    // Read holding registers
    uint16_t data[10];
    Modbus::StatusCode status = client.readHoldingRegisters(0, 10, data);
    
    if (Modbus::StatusIsGood(status))
    {
        std::cout << "Data read successfully" << std::endl;
    }
    else
    {
        std::cerr << "Error: " << port.lastErrorText() << std::endl;
    }
    
    return 0;
}
```

### Non-Blocking Client Example {#non-blocking-client-example}

```cpp
#include <iostream>
#include "ModbusClient.h"
#include "ModbusClientPort.h"
#include "ModbusTcpPort.h"
#include "Modbus.h"

int main()
{
    ModbusTcpPort *tcp = new ModbusTcpPort();
    tcp->setHost("192.168.1.100");
    ModbusClientPort port(tcp);
    ModbusClient client(1, &port);
    
    uint16_t data[10];
    
    // Non-blocking operation
    while (true)
    {
        Modbus::StatusCode status = client.readHoldingRegisters(0, 10, data);
        
        if (Modbus::StatusIsGood(status))
        {
            std::cout << "Data read successfully" << std::endl;
            break;
        }
        else if (Modbus::StatusIsProcessing(status))
        {
            // Do other work while waiting
            // ...
            Modbus::msleep(1);
        }
        else
        {
            std::cerr << "Error: " << port.lastErrorText() << std::endl;
            break;
        }
    }
    
    return 0;
}
```
