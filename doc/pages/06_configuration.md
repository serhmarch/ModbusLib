# Configuration and Settings Reference {#configuration}

[TOC]

## Overview {#configuration-overview}

This document provides a comprehensive reference for configuring ModbusLib components. ModbusLib offers extensive configuration options for TCP, RTU, and ASCII protocol implementations, supporting both client and server modes with flexible timeout management, connection settings, and protocol-specific parameters.

All configuration in ModbusLib follows a consistent pattern:
- **Settings Structures**: Use `Modbus::SerialSettings` and `Modbus::TcpSettings` for initial configuration
- **Defaults Classes**: Each port class provides a `Defaults` nested class with recommended default values
- **Setter Methods**: Runtime configuration through dedicated setter methods (e.g., `setTimeout()`, `setBaudRate()`)
- **Factory Functions**: Convenient port creation via `Modbus::createPort()`, `Modbus::createClientPort()`, and `Modbus::createServerPort()`

### Key Configuration Concepts {#key-configuration-concepts}

1. **Blocking vs. Non-Blocking Mode**: Determines whether operations wait for completion or return immediately with `Status_Processing`
2. **Timeout Management**: Controls how long operations wait for responses or data
3. **Protocol-Specific Settings**: Each protocol (TCP, RTU, ASCII) has unique configuration requirements
4. **Client vs. Server Configuration**: Different settings apply depending on operational mode
5. **Qt Integration**: Enhanced configuration support when using Qt framework

---

## Common Port Settings {#common-port-settings}

All Modbus port types share common configuration options that control basic operational behavior.

### Blocking Mode {#config-blocking-mode}

Determines the operational mode of the port:

| Mode | Description | Use Case |
|------|-------------|----------|
| **Blocking** (`true`) | Operations wait until completion or timeout | Simple single-threaded applications |
| **Non-Blocking** (`false`) | Operations return immediately with `Status_Processing` | Event-driven architectures, GUI applications |

**Configuration:**
```cpp
// Set at construction
ModbusTcpPort port(true);  // Blocking mode
ModbusRtuPort port(false); // Non-blocking mode

// Check current mode
bool isBlocking = port.isBlocking();
bool isNonBlocking = port.isNonBlocking();
```

**Non-Blocking Usage Pattern:**
```cpp
ModbusTcpPort port(false);
Modbus::StatusCode status;

if (Modbus::StatusIsGood(status)) {
    // Port opened successfully
}
```

### Timeout Settings {#timeout-settings}

Timeout values control how long operations wait for responses. Units are in **milliseconds**.

| Parameter | Description | Typical Range |
|-----------|-------------|---------------|
| `timeout` | Connection/read timeout for TCP and serial first byte | 1000-5000 ms |
| `timeoutInterByte` | Serial inter-byte timeout (RTU/ASCII only) | 10-100 ms |

**Default Values:**
- TCP timeout: **3000 ms**
- Serial first byte timeout: **1000 ms**
- Serial inter-byte timeout: **50 ms**

---

## TCP Port Configuration {#tcp-port-configuration}

TCP-based Modbus communication operates over TCP/IP networks, typically on port 502 (standard Modbus port).

### Settings Parameters {#tcp-settings-parameters}

#### Host Address {#host-address}

The IP address or DNS hostname of the remote Modbus device.

**Type:** `const Modbus::Char *` (C-string)  
**Default:** `"127.0.0.1"` (localhost)  
**Examples:** `"192.168.1.100"`, `"modbus.example.com"`, `"localhost"`

```cpp
ModbusTcpPort port;
port.setHost("192.168.1.50");
const Modbus::Char *host = port.host(); // Get current host
```

#### TCP Port Number {#tcp-port-number}

The TCP port number on which the remote Modbus server listens.

**Type:** `uint16_t`  
**Default:** `502` (`Modbus::STANDARD_TCP_PORT`)  
**Range:** 1-65535  
**Common Values:** 502 (standard), 10502 (alternative)

```cpp
ModbusTcpPort port;
port.setPort(502);
uint16_t portNum = port.port(); // Get current port
```

#### Connection Timeout {#tcp-connection-timeout}

Maximum time to wait for connection establishment and response reception.

**Type:** `uint32_t`  
**Default:** `3000` milliseconds  
**Range:** 100-30000 ms (typical)  
**Units:** milliseconds

```cpp
ModbusTcpPort port;
port.setTimeout(5000); // 5 second timeout
uint32_t timeout = port.timeout();
```

### Default Values Class {#tcp-default-values}

```cpp
ModbusTcpPort::Defaults::Defaults() :
    host   ("127.0.0.1"),
    port   (502),
    timeout(3000)
{
}

// Access defaults
const ModbusTcpPort::Defaults &defaults = ModbusTcpPort::Defaults::instance();
```

### TCP Configuration Structure {#tcp-configuration-structure}

For use with factory functions:

```cpp
Modbus::TcpSettings settings;
settings.host    = "192.168.1.100";
settings.port    = 502;
settings.timeout = 5000;
settings.maxconn = 10; // Server only

// Create port with settings
ModbusPort *port = Modbus::createPort(Modbus::TCP, &settings, false);
```

### Complete TCP Client Example {#config-tcp-client-example}

```cpp
#include <ModbusTcpPort.h>
#include <ModbusClientPort.h>

// Method 1: Direct configuration
ModbusTcpPort *tcpPort = new ModbusTcpPort(false); // Non-blocking
tcpPort->setHost("192.168.1.50");
tcpPort->setPort(502);
tcpPort->setTimeout(5000);

ModbusClientPort *client = new ModbusClientPort(tcpPort);
client->setTries(3);
client->setBroadcastEnabled(true);

// Method 2: Using settings structure
Modbus::TcpSettings settings;
settings.host    = "192.168.1.50";
settings.port    = 502;
settings.timeout = 5000;

ModbusClientPort *client2 = Modbus::createClientPort(Modbus::TCP, &settings, false);

// Open and use
Modbus::StatusCode status;
do {
    status = client->readHoldingRegisters(0, 10, registers);
    Modbus::msleep(1);
} while (Modbus::StatusIsProcessing(status));

```

### Transaction ID Management {#tcp-transaction-id}

TCP protocol uses transaction IDs to match requests with responses.

```cpp
ModbusTcpPort port;

// Auto-increment mode (default: enabled)
bool autoInc = port.autoIncrement(); // Returns true by default

// Current transaction ID
uint16_t txId = port.transactionId();

// Repeat transaction ID for retry
port.setNextRequestRepeated(true); // Next request uses same ID
```

---

## RTU Port Configuration {#rtu-port-configuration}

RTU (Remote Terminal Unit) is a compact binary protocol for serial communication using RS-232, RS-485, or RS-422.

### Serial Port Settings {#serial-port-settings}

#### Port Name {#port-name}

Operating system-specific serial port identifier.

**Type:** `const Modbus::Char *` (C-string)  
**Default:** `"COM1"` (Windows) or `"/dev/ttyS0"` (Unix/Linux)  
**Examples:**
- Windows: `"COM1"`, `"COM2"`, `"COM10"`
- Linux: `"/dev/ttyS0"`, `"/dev/ttyUSB0"`, `"/dev/ttyAMA0"`
- macOS: `"/dev/cu.usbserial-1420"`, `"/dev/tty.usbserial-1420"`

```cpp
ModbusRtuPort port;
port.setPortName("COM3");
const Modbus::Char *name = port.portName();
```

#### Baud Rate {#baud-rate}

Serial communication speed in bits per second.

**Type:** `int32_t`  
**Default:** `9600`  
**Common Values:** 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200

```cpp
ModbusRtuPort port;
port.setBaudRate(19200);
int32_t baud = port.baudRate();
```

#### Data Bits {#data-bits}

Number of data bits in each character.

**Type:** `int8_t`  
**Default:** `8`  
**Range:** 5, 6, 7, 8  
**Standard:** 8 bits for RTU

```cpp
ModbusRtuPort port;
port.setDataBits(8);
int8_t bits = port.dataBits();
```

#### Parity {#parity}

Error detection scheme for serial communication.

**Type:** `Modbus::Parity` (enum)  
**Default:** `Modbus::NoParity`  
**Values:**

| Value | Description | Usage |
|-------|-------------|-------|
| `Modbus::NoParity` | No parity bit | Most common for RTU |
| `Modbus::EvenParity` | Even parity | Error detection |
| `Modbus::OddParity` | Odd parity | Ensures state transition |
| `Modbus::SpaceParity` | Space parity (always 0) | Rarely used |
| `Modbus::MarkParity` | Mark parity (always 1) | Rarely used |

```cpp
ModbusRtuPort port;
port.setParity(Modbus::NoParity);
Modbus::Parity parity = port.parity();

// String conversion
const Modbus::Char *parityStr = Modbus::sparity(parity); // "NoParity"
Modbus::Parity p = Modbus::toparity("EvenParity");
```

#### Stop Bits {#stop-bits}

Number of stop bits after each character.

**Type:** `Modbus::StopBits` (enum)  
**Default:** `Modbus::OneStop`  
**Values:**

| Value | Description | Usage |
|-------|-------------|-------|
| `Modbus::OneStop` | 1 stop bit | Standard for most applications |
| `Modbus::OneAndHalfStop` | 1.5 stop bits | Special cases |
| `Modbus::TwoStop` | 2 stop bits | Increased reliability |

```cpp
ModbusRtuPort port;
port.setStopBits(Modbus::OneStop);
Modbus::StopBits stops = port.stopBits();

// String conversion
const Modbus::Char *stopStr = Modbus::sstopBits(stops); // "OneStop"
Modbus::StopBits s = Modbus::tostopBits("TwoStop");
```

#### Flow Control {#flow-control}

Hardware or software flow control mechanism.

**Type:** `Modbus::FlowControl` (enum)  
**Default:** `Modbus::NoFlowControl`  
**Values:**

| Value | Description | Usage |
|-------|-------------|-------|
| `Modbus::NoFlowControl` | No flow control | Standard for RTU/ASCII |
| `Modbus::HardwareControl` | RTS/CTS hardware control | Long cables, high baud |
| `Modbus::SoftwareControl` | XON/XOFF software control | Rarely used |

```cpp
ModbusRtuPort port;
port.setFlowControl(Modbus::NoFlowControl);
Modbus::FlowControl flow = port.flowControl();
```

#### Timeout First Byte {#timeout-first-byte}

Maximum time to wait for the first byte of a response.

**Type:** `uint32_t`  
**Default:** `1000` milliseconds  
**Range:** 100-10000 ms (typical)  
**Units:** milliseconds

```cpp
ModbusRtuPort port;
port.setTimeoutFirstByte(2000); // 2 seconds
uint32_t timeout = port.timeoutFirstByte();
```

#### Timeout Inter Byte {#timeout-inter-byte}

Maximum time between consecutive bytes within a frame.

**Type:** `uint32_t`  
**Default:** `50` milliseconds  
**Range:** 10-200 ms (typical)  
**Units:** milliseconds

**Note:** RTU protocol requires 3.5 character times of silence between frames, typically calculated from baud rate.

```cpp
ModbusRtuPort port;
port.setTimeoutInterByte(100); // 100 ms
uint32_t timeout = port.timeoutInterByte();
```

### Default Values Class {#serial-default-values}

```cpp
ModbusSerialPort::Defaults::Defaults() :
    portName        ("COM1"),  // Windows
    baudRate        (9600),
    dataBits        (8),
    parity          (Modbus::NoParity),
    stopBits        (Modbus::OneStop),
    flowControl     (Modbus::NoFlowControl),
    timeoutFirstByte(1000),
    timeoutInterByte(50)
{
}

// Access defaults
const ModbusSerialPort::Defaults &defaults = ModbusSerialPort::Defaults::instance();
```

### RTU Configuration Structure {#rtu-configuration-structure}

For use with factory functions:

```cpp
Modbus::SerialSettings settings;
settings.portName         = "COM3";
settings.baudRate         = 19200;
settings.dataBits         = 8;
settings.parity           = Modbus::NoParity;
settings.stopBits         = Modbus::OneStop;
settings.flowControl      = Modbus::NoFlowControl;
settings.timeoutFirstByte = 1000;
settings.timeoutInterByte = 50;

// Create RTU port with settings
ModbusPort *port = Modbus::createPort(Modbus::RTU, &settings, false);
```

### Complete RTU Client Example {#config-rtu-client-example}

```cpp
#include <ModbusRtuPort.h>
#include <ModbusClientPort.h>

// Method 1: Direct configuration
ModbusRtuPort *rtuPort = new ModbusRtuPort(true); // Blocking mode
rtuPort->setPortName("/dev/ttyUSB0");
rtuPort->setBaudRate(19200);
rtuPort->setDataBits(8);
rtuPort->setParity(Modbus::NoParity);
rtuPort->setStopBits(Modbus::OneStop);
rtuPort->setFlowControl(Modbus::NoFlowControl);
rtuPort->setTimeoutFirstByte(2000);
rtuPort->setTimeoutInterByte(100);

ModbusClientPort *client = new ModbusClientPort(rtuPort);
client->setTries(3);

// Method 2: Using settings structure
Modbus::SerialSettings settings;
settings.portName         = "/dev/ttyUSB0";
settings.baudRate         = 19200;
settings.dataBits         = 8;
settings.parity           = Modbus::NoParity;
settings.stopBits         = Modbus::OneStop;
settings.flowControl      = Modbus::NoFlowControl;
settings.timeoutFirstByte = 2000;
settings.timeoutInterByte = 100;

ModbusClientPort *client2 = Modbus::createClientPort(Modbus::RTU, &settings, true);

// Open and use
Modbus::StatusCode status = client->readHoldingRegisters(1, 0, 10, registers);
```

### Common RTU Configurations {#common-rtu-configurations}

#### Standard Industrial (9600 8N1) {#standard-industrial-9600-8n1}
```cpp
settings.baudRate = 9600;
settings.dataBits = 8;
settings.parity   = Modbus::NoParity;
settings.stopBits = Modbus::OneStop;
```

#### High-Speed (115200 8N1) {#high-speed-115200-8n1}
```cpp
settings.baudRate = 115200;
settings.dataBits = 8;
settings.parity   = Modbus::NoParity;
settings.stopBits = Modbus::OneStop;
settings.timeoutInterByte = 1; // Shorter timeout for high speed
```

#### Legacy with Parity (9600 8E1) {#legacy-with-parity-9600-8e1}
```cpp
settings.baudRate = 9600;
settings.dataBits = 8;
settings.parity   = Modbus::EvenParity;
settings.stopBits = Modbus::OneStop;
```

---

## ASCII Port Configuration {#ascii-port-configuration}

ASCII protocol uses the same serial port settings as RTU but with ASCII hexadecimal encoding and LRC checksums.

### Serial Settings {#ascii-serial-settings}

ASCII shares all serial port configuration parameters with RTU:
- Port name, baud rate, data bits, parity, stop bits, flow control
- Timeout first byte, timeout inter-byte

**Key Difference:** ASCII is human-readable (hex characters) and uses `:` start and `CR-LF` end delimiters.

### Default Values {#ascii-default-values}

Same as RTU - see [RTU Port Configuration](#rtu-port-configuration) for complete details.

### ASCII Configuration Structure {#ascii-configuration-structure}

```cpp
Modbus::SerialSettings settings;
settings.portName         = "/dev/ttyS0";
settings.baudRate         = 9600;
settings.dataBits         = 7;          // Often 7 bits for ASCII
settings.parity           = Modbus::EvenParity; // Often Even parity
settings.stopBits         = Modbus::OneStop;
settings.flowControl      = Modbus::NoFlowControl;
settings.timeoutFirstByte = 1000;
settings.timeoutInterByte = 50;

// Create ASCII port with settings
ModbusPort *port = Modbus::createPort(Modbus::ASC, &settings, false);
```

### Complete ASCII Client Example {#ascii-client-example}

```cpp
#include <ModbusAscPort.h>
#include <ModbusClientPort.h>

// ASCII configuration (often uses 7E1)
ModbusAscPort *ascPort = new ModbusAscPort(true); // Blocking
ascPort->setPortName("COM1");
ascPort->setBaudRate(9600);
ascPort->setDataBits(7);              // 7 bits common for ASCII
ascPort->setParity(Modbus::EvenParity); // Even parity common
ascPort->setStopBits(Modbus::OneStop);
ascPort->setFlowControl(Modbus::NoFlowControl);
ascPort->setTimeoutFirstByte(1000);
ascPort->setTimeoutInterByte(50);

ModbusClientPort *client = new ModbusClientPort(ascPort);
client->setTries(3);

// Open and use
Modbus::StatusCode status = client.readHoldingRegisters(0, 10, registers);
```

### Common ASCII Configurations {#common-ascii-configurations}

#### Standard ASCII (9600 7E1) {#standard-ascii-9600-7e1}
```cpp
settings.baudRate = 9600;
settings.dataBits = 7;
settings.parity   = Modbus::EvenParity;
settings.stopBits = Modbus::OneStop;
```

#### Alternative ASCII (9600 8N1) {#alternative-ascii-9600-8n1}
```cpp
settings.baudRate = 9600;
settings.dataBits = 8;
settings.parity   = Modbus::NoParity;
settings.stopBits = Modbus::OneStop;
```

---

## Client Port Configuration {#client-port-configuration}

`ModbusClientPort` wraps any protocol port and provides client-side Modbus function execution with retry and resource sharing capabilities.

### Retry/Tries Count {#client-retry-count}

Number of times to retry a failed Modbus request.

**Type:** `uint32_t`  
**Default:** `1` (no retries)  
**Range:** 1-10 (typical)  
**Property:** `tries()` / `setTries()`  
**Legacy:** `repeatCount()` / `setRepeatCount()` (same functionality)

```cpp
ModbusClientPort client(port);
client.setTries(3); // Try up to 3 times on failure

uint32_t tries = client.tries();
uint32_t count = client.repeatCount(); // Same as tries()
```

**Behavior:**
- `tries = 1`: Single attempt, no retries
- `tries = 3`: Up to 3 attempts (initial + 2 retries)
- Failed requests return last error status

### Broadcast Mode {#client-broadcast-mode}

Enable/disable broadcast mode for unit address 0.

**Type:** `bool`  
**Default:** `true` (enabled)  
**Property:** `isBroadcastEnabled()` / `setBroadcastEnabled()`

```cpp
ModbusClientPort client(port);
client.setBroadcastEnabled(true); // Enable broadcast

bool enabled = client.isBroadcastEnabled();

// Broadcast write (unit 0, no response expected)
client.writeSingleRegister(0, 100, 1234);
```

**Note:** Modbus specification defines unit address 0 as broadcast. When enabled, requests to unit 0 don't expect responses.

### Complete Client Configuration Example {#complete-client-example}

```cpp
#include <ModbusClientPort.h>
#include <ModbusTcpPort.h>

// Create and configure TCP port
Modbus::TcpSettings tcpSettings;
tcpSettings.host    = "192.168.1.100";
tcpSettings.port    = 502;
tcpSettings.timeout = 5000;

ModbusTcpPort *tcpPort = new ModbusTcpPort(false);
tcpPort->setHost(tcpSettings.host);
tcpPort->setPort(tcpSettings.port);
tcpPort->setTimeout(tcpSettings.timeout);

// Create and configure client
ModbusClientPort *client = new ModbusClientPort(tcpPort);
client->setTries(3);              // Retry up to 3 times
client->setBroadcastEnabled(true); // Enable broadcast

// Read holding registers
uint16_t regs[10];
status = client->readHoldingRegisters(1, 0, 10, regs);

// Check statistics
uint32_t actualTries = client->lastTries();
Modbus::StatusCode lastStatus = client->lastStatus();
const Modbus::Char *errorText = client->lastErrorText();

// Cleanup
client->close();
delete client; // Deletes tcpPort automatically
```

---

## Server Configuration {#server-configuration}

### TCP Server Configuration {#tcp-server-configuration}

`ModbusTcpServer` manages multiple client connections on a TCP port.

The TCP server exposes settings to control listening behavior and connection handling.

- **Bind Address**: `ipaddr` — local IP address to bind the server.
    - Examples: `127.0.0.1` (loopback), `0.0.0.0` (all interfaces), specific NIC IP like `192.168.1.10`.
- **Port**: `port` — TCP port number (default `STANDARD_TCP_PORT`, usually `502`).
- **Timeout**: `timeout` — read timeout per connection (ms).
- **Max Connections**: `maxconn` — maximum simultaneous connections.

```cpp
ModbusTcpServer server(device);
server.setIpaddr("0.0.0.0");
server.setPort(50200);
server.setTimeout(10000);
server.setMaxConnections(8);
```

Alternatively, use `Modbus::TcpSettings`:

```cpp
Modbus::TcpSettings ts;
ts.ipaddr = "127.0.0.1";
ts.port = STANDARD_TCP_PORT;
ts.timeout = 7000;
ts.maxconn = 16;
```
#### Bind Address {#tcp-server-ipaddr}

Local IP address for the server to bind.

**Type:** `const Char *` (C-string)  
**Default:** `"0.0.0.0"` (all interfaces)  
**Property:** `ipaddr()` / `setIpaddr()`

```cpp
ModbusTcpServer server(device);
server.setIpaddr("0.0.0.0");
const Char *addr = server.ipaddr();
``` 

#### TCP Port Number {#tcp-server-port}

Port on which the server listens for incoming connections.

**Type:** `uint16_t`  
**Default:** `502` (`Modbus::STANDARD_TCP_PORT`)  
**Property:** `port()` / `setPort()`

```cpp
ModbusTcpServer server(device);
server.setPort(502);
uint16_t port = server.port();
```

#### Connection Timeout {#tcp-server-timeout}

Timeout for each individual client connection.

**Type:** `uint32_t`  
**Default:** `3000` milliseconds  
**Property:** `timeout()` / `setTimeout()`  
**Units:** milliseconds

```cpp
ModbusTcpServer server(device);
server.setTimeout(5000); // 5 second timeout per connection
uint32_t timeout = server.timeout();
```

#### Maximum Connections {#tcp-server-max-connections}

Maximum number of simultaneous client connections.

**Type:** `uint32_t`  
**Default:** `10`  
**Range:** 1-1000 (typical)  
**Property:** `maxConnections()` / `setMaxConnections()`

```cpp
ModbusTcpServer server(device);
server.setMaxConnections(20); // Allow 20 concurrent clients
uint32_t maxConn = server.maxConnections();
```

#### Broadcast Mode {#tcp-server-broadcast}

Enable handling of broadcast requests (unit 0).

**Type:** `bool`  
**Default:** `true`  
**Property:** `isBroadcastEnabled()` / `setBroadcastEnabled()`

```cpp
ModbusTcpServer server(device);
server.setBroadcastEnabled(true);
bool enabled = server.isBroadcastEnabled();
```

#### Unit Map {#tcp-server-unit-map}

Filter which unit addresses the server responds to.

**Type:** `const void *` (32-byte bitmap)  
**Property:** `unitMap()` / `setUnitMap()`

```cpp
ModbusTcpServer server(device);

// Enable units 1, 2, 5
uint8_t unitmap[MB_UNITMAP_SIZE] = {0};
MB_UNITMAP_SET_BIT(unitmap, 1, true);
MB_UNITMAP_SET_BIT(unitmap, 2, true);
MB_UNITMAP_SET_BIT(unitmap, 5, true);

server.setUnitMap(unitmap);
const uint8_t *map = server.unitMap();
```

#### TCP Server Defaults Class {#tcp-server-defaults}

```cpp
ModbusTcpServer::Defaults::Defaults() :
    port   (502),   // Modbus::STANDARD_TCP_PORT
    timeout(3000),
    maxconn(10)
{
}

const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
```

#### TCP Server Configuration Structure {#tcp-server-structure}

```cpp
Modbus::TcpSettings settings;
settings.host    = nullptr;    // Not used for server
settings.port    = 502;        // Listening port
settings.timeout = 5000;       // Per-connection timeout
settings.maxconn = 20;         // Max simultaneous connections

ModbusServerPort *server = Modbus::createServerPort(&device, Modbus::TCP, &settings);
```

#### Complete TCP Server Example {#config-tcp-server-example}

```cpp
#include <ModbusTcpServer.h>

// Implement device interface
class MyDevice : public ModbusInterface {
public:
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset, 
                                           uint16_t count, uint16_t *values) override {
        for (uint16_t i = 0; i < count; i++)
            values[i] = m_registers[offset + i];
        return Modbus::Status_Good;
    }
    // Implement other methods...
private:
    uint16_t m_registers[1000];
};

MyDevice device;

// Create and configure server
ModbusTcpServer server(&device);
server.setPort(502);
server.setTimeout(5000);
server.setMaxConnections(10);
server.setBroadcastEnabled(true);

// Optional: Set unit filter
uint8_t unitmap[MB_UNITMAP_SIZE] = {0};
MB_UNITMAP_SET_BIT(unitmap, 1, true); // Only respond to unit 1
server.setUnitMap(unitmap);

// Main server loop
while (running) {
    server.process(); // Handle all connections
    Modbus::msleep(1);
}
server.close();
```

### Serial Server Configuration {#serial-server-configuration}

`ModbusServerPort` (with serial port) handles serial-based server functionality.

#### Serial Settings {#serial-server-settings}

Uses same serial port configuration as client:
- Port name, baud rate, data bits, parity, stop bits, flow control
- Timeout first byte, timeout inter-byte

See [RTU Port Configuration](#rtu-port-configuration) for complete details.

#### Broadcast and Unit Map {#serial-server-broadcast-unitmap}

Same as TCP server:
- `setBroadcastEnabled(bool)` - Enable broadcast handling
- `setUnitMap(const void*)` - Filter unit addresses

#### Complete Serial Server Example {#serial-server-example}

```cpp
#include <ModbusServerPort.h>
#include <ModbusRtuPort.h>

// Implement device interface
class MyDevice : public ModbusInterface {
    // ... implement interface methods
};

MyDevice device;

// Configure serial port
Modbus::SerialSettings settings;
settings.portName         = "/dev/ttyUSB0";
settings.baudRate         = 19200;
settings.dataBits         = 8;
settings.parity           = Modbus::NoParity;
settings.stopBits         = Modbus::OneStop;
settings.flowControl      = Modbus::NoFlowControl;
settings.timeoutFirstByte = 1000;
settings.timeoutInterByte = 50;

// Create serial server
ModbusServerPort *server = Modbus::createServerPort(&device, Modbus::RTU, &settings, false);
server->setBroadcastEnabled(true);

// Main server loop
while (running) {
    server->process();
    Modbus::msleep(1);
}
server->close();

delete server;
```

---

## Qt-Specific Configuration {#qt-configuration}

When ModbusLib is built with Qt support (`QT_CORE_LIB` defined),
additional configuration options become available through Qt's `QVariantMap` settings.

### Qt Settings Map {#qt-settings-map}

Use Qt's `Modbus::Settings` (alias for `QVariantMap`) for flexible configuration:

```cpp
#include <ModbusQt.h>

Modbus::Settings settings;
settings[Modbus::Strings::instance().unit]   = 1;
settings[Modbus::Strings::instance().type]   = Modbus::TCP;
settings[Modbus::Strings::instance().host]   = QString("192.168.1.100");
settings[Modbus::Strings::instance().port]   = 502;
settings[Modbus::Strings::instance().timeout] = 5000;
```

### Qt String Constants {#qt-string-constants}

The `Modbus::Strings` class provides string constants for settings keys:

```cpp
const Modbus::Strings &s = Modbus::Strings::instance();

// General settings
s.unit              // "unit"
s.type              // "type"
s.tries             // "tries"

// TCP settings
s.host              // "host"
s.port              // "port"
s.timeout           // "timeout"
s.maxconn           // "maxconn"

// Serial settings
s.serialPortName    // "serialPortName"
s.baudRate          // "baudRate"
s.dataBits          // "dataBits"
s.parity            // "parity"
s.stopBits          // "stopBits"
s.flowControl       // "flowControl"
s.timeoutFirstByte  // "timeoutFirstByte"
s.timeoutInterByte  // "timeoutInterByte"

// Enum string values
s.TCP, s.RTU, s.ASC // Protocol types
s.NoParity, s.EvenParity, s.OddParity // Parity values
s.OneStop, s.TwoStop // Stop bits
s.NoFlowControl, s.HardwareControl // Flow control
```

### Qt Defaults Class {#qt-defaults}

```cpp
Modbus::Defaults::Defaults() :
    unit              (1),
    type              (Modbus::TCP),
    tries             (1),
    host              (ModbusTcpPort::Defaults::instance().host),
    port              (ModbusTcpPort::Defaults::instance().port),
    timeout           (ModbusTcpPort::Defaults::instance().timeout),
    maxconn           (ModbusTcpServer::Defaults::instance().maxconn),
    serialPortName    (ModbusSerialPort::Defaults::instance().portName),
    baudRate          (ModbusSerialPort::Defaults::instance().baudRate),
    dataBits          (ModbusSerialPort::Defaults::instance().dataBits),
    parity            (ModbusSerialPort::Defaults::instance().parity),
    stopBits          (ModbusSerialPort::Defaults::instance().stopBits),
    flowControl       (ModbusSerialPort::Defaults::instance().flowControl),
    timeoutFirstByte  (ModbusSerialPort::Defaults::instance().timeoutFirstByte),
    timeoutInterByte  (ModbusSerialPort::Defaults::instance().timeoutInterByte),
    isBroadcastEnabled(true)
{
}

const Modbus::Defaults &defaults = Modbus::Defaults::instance();
```

### Qt Settings Getters {#qt-settings-getters}

Helper functions extract settings with type conversion and validation:

```cpp
uint8_t unit = Modbus::getSettingUnit(settings);
Modbus::ProtocolType type = Modbus::getSettingType(settings);
QString host = Modbus::getSettingHost(settings);
uint16_t port = Modbus::getSettingPort(settings);
uint32_t timeout = Modbus::getSettingTimeout(settings);
// ... additional getters for all settings

// With error checking
bool ok;
uint32_t tries = Modbus::getSettingTries(settings, &ok);
if (ok) {
    // Value was present and valid
}
```

### Complete Qt Configuration Example {#qt-configuration-example}

```cpp
#include <ModbusQt.h>
#include <ModbusClient.h>
#include <ModbusClientPort.h>

// Method 1: Create TCP client port using QVariantMap
Modbus::Settings settings;
const Modbus::Strings &s = Modbus::Strings::instance();

settings[s.unit]    = 1;
settings[s.type]    = s.TCP;
settings[s.host]    = QString("192.168.1.100");
settings[s.port]    = 502;
settings[s.timeout] = 5000;
settings[s.tries]   = 3;

// Create client port with settings
ModbusClientPort *clientPort = Modbus::createClientPort(settings, false);
clientPort->setTries(Modbus::getSettingTries(settings));

// Create client bound to unit 1
uint8_t unit = Modbus::getSettingUnit(settings);
ModbusClient client(unit, clientPort);

// Open and use
uint16_t registers[10];
do {
    status = client.readHoldingRegisters(0, 10, registers);
    Modbus::msleep(1);
} while (Modbus::StatusIsProcessing(status));

// Method 2: RTU configuration with helper functions
Modbus::Settings rtuSettings;
Modbus::setSettingType(rtuSettings, Modbus::RTU);
Modbus::setSettingSerialPortName(rtuSettings, QString("/dev/ttyUSB0"));
Modbus::setSettingBaudRate(rtuSettings, 19200);
Modbus::setSettingDataBits(rtuSettings, 8);
Modbus::setSettingParity(rtuSettings, Modbus::NoParity);
Modbus::setSettingStopBits(rtuSettings, Modbus::OneStop);
Modbus::setSettingFlowControl(rtuSettings, Modbus::NoFlowControl);
Modbus::setSettingTimeoutFirstByte(rtuSettings, 2000);
Modbus::setSettingTimeoutInterByte(rtuSettings, 100);
Modbus::setSettingTries(rtuSettings, 3);

// Create RTU client
ModbusClientPort *rtuPort = Modbus::createClientPort(rtuSettings, true);
ModbusClient rtuClient(Modbus::getSettingUnit(rtuSettings), rtuPort);

// Method 3: Using string constants directly
Modbus::Settings tcpSettings;
tcpSettings[s.type]             = Modbus::enumKey<Modbus::ProtocolType>(Modbus::TCP);
tcpSettings[s.host]             = QString("192.168.1.100");
tcpSettings[s.port]             = 502;
tcpSettings[s.timeout]          = 5000;
tcpSettings[s.tries]            = 3;

ModbusClientPort *tcpPort = Modbus::createClientPort(tcpSettings, false);
ModbusClient tcpClient(1, tcpPort);
```

---

## Default Values Reference {#default-values-reference}

### Summary Table {#default-values-summary}

| Port Type | Parameter | Default Value | Units |
|-----------|-----------|---------------|-------|
| **TCP** | host | `"127.0.0.1"` | - |
| **TCP** | port | `502` | - |
| **TCP** | timeout | `3000` | ms |
| **TCP Server** | maxconn | `10` | connections |
| **Serial** | portName | `"COM1"` (Win) / `"/dev/ttyS0"` (Unix) | - |
| **Serial** | baudRate | `9600` | bps |
| **Serial** | dataBits | `8` | bits |
| **Serial** | parity | `Modbus::NoParity` | - |
| **Serial** | stopBits | `Modbus::OneStop` | - |
| **Serial** | flowControl | `Modbus::NoFlowControl` | - |
| **Serial** | timeoutFirstByte | `1000` | ms |
| **Serial** | timeoutInterByte | `50` | ms |
| **Client** | tries | `1` | count |
| **Client** | broadcastEnabled | `true` | - |
| **Server** | broadcastEnabled | `true` | - |

### Accessing Defaults in Code {#accessing-defaults}

All port classes provide static `Defaults` classes:

```cpp
// TCP defaults
const ModbusTcpPort::Defaults &tcpDef = ModbusTcpPort::Defaults::instance();
const char *host = tcpDef.host;       // "127.0.0.1"
uint16_t port = tcpDef.port;          // 502
uint32_t timeout = tcpDef.timeout;    // 3000

// Serial defaults
const ModbusSerialPort::Defaults &serDef = ModbusSerialPort::Defaults::instance();
const char *portName = serDef.portName;         // "COM1"
int32_t baudRate = serDef.baudRate;             // 9600
int8_t dataBits = serDef.dataBits;              // 8
Modbus::Parity parity = serDef.parity;          // Modbus::NoParity
Modbus::StopBits stopBits = serDef.stopBits;    // Modbus::OneStop
uint32_t tfb = serDef.timeoutFirstByte;         // 1000
uint32_t tib = serDef.timeoutInterByte;         // 50

// TCP Server defaults
const ModbusTcpServer::Defaults &srvDef = ModbusTcpServer::Defaults::instance();
uint16_t port = srvDef.port;       // 502
uint32_t timeout = srvDef.timeout; // 3000
uint32_t maxconn = srvDef.maxconn; // 10

// Qt defaults (when available)
#ifdef QT_CORE_LIB
const Modbus::Defaults &qtDef = Modbus::Defaults::instance();
uint8_t unit = qtDef.unit;                    // 1
Modbus::ProtocolType type = qtDef.type;       // Modbus::TCP
uint32_t tries = qtDef.tries;                 // 1
bool broadcast = qtDef.isBroadcastEnabled;    // true
#endif
```

---

## Configuration Examples {#configuration-examples}

### Example 1: Simple TCP Client (Blocking) {#example-tcp-client-blocking}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>

int main() {
    // Create TCP client (blocking mode)
    Modbus::TcpSettings settings;
    settings.host    = "192.168.1.50";
    settings.port    = 502;
    settings.timeout = 5000;
    
    ModbusClientPort *client = Modbus::createClientPort(Modbus::TCP, &settings, true);
    client->setTries(3);
    
    // Read
    uint16_t registers[10];
    Modbus::StatusCode status = client->readHoldingRegisters(1, 0, 10, registers);
    
    if (Modbus::StatusIsGood(status)) {
        // Process registers
        for (int i = 0; i < 10; i++) {
            printf("Register %d: %u\n", i, registers[i]);
        }
    }
    client->close();
    
    delete client;
    return 0;
}
```

### Example 2: RTU Client with Custom Serial Settings {#example-rtu-custom-settings}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>

int main() {
    // High-speed RTU configuration
    Modbus::SerialSettings settings;
    settings.portName         = "/dev/ttyUSB0";
    settings.baudRate         = 115200;
    settings.dataBits         = 8;
    settings.parity           = Modbus::NoParity;
    settings.stopBits         = Modbus::OneStop;
    settings.flowControl      = Modbus::NoFlowControl;
    settings.timeoutFirstByte = 500;  // Faster timeout
    settings.timeoutInterByte = 1;    // Short inter-byte
    
    ModbusClientPort *client = Modbus::createClientPort(Modbus::RTU, &settings, true);
    client->setTries(3);
    
    uint16_t value = 1234;
    Modbus::StatusCode status = client->writeSingleRegister(1, 100, value);    
    delete client;
    return 0;
}
```

### Example 3: Non-Blocking TCP Client {#example-tcp-nonblocking}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>

int main() {
    Modbus::TcpSettings settings;
    settings.host    = "192.168.1.50";
    settings.port    = 502;
    settings.timeout = 5000;
    
    // Non-blocking mode
    ModbusClientPort *client = Modbus::createClientPort(Modbus::TCP, &settings, false);
    
    // Non-blocking read
    uint16_t registers[10];
    Modbus::StatusCode status;
    do {
        status = client->readHoldingRegisters(1, 0, 10, registers);
        Modbus::msleep(10);
    } while (Modbus::StatusIsProcessing(status));
    
    if (Modbus::StatusIsGood(status)) {
        // Process registers
    }
        
    delete client;
    return 0;
}
```

### Example 4: TCP Server with Multiple Connections {#example-tcp-server-multi}

```cpp
#include <Modbus.h>
#include <ModbusTcpServer.h>

class MyDevice : public ModbusInterface {
public:
    MyDevice() { memset(m_registers, 0, sizeof(m_registers)); }
    
    Modbus::StatusCode readHoldingRegisters(uint8_t unit, uint16_t offset,
                                           uint16_t count, uint16_t *values) override {
        if (offset + count > 1000) return Modbus::Status_BadIllegalDataAddress;
        memcpy(values, &m_registers[offset], count * sizeof(uint16_t));
        return Modbus::Status_Good;
    }
    
    Modbus::StatusCode writeSingleRegister(uint8_t unit, uint16_t offset,
                                          uint16_t value) override {
        if (offset >= 1000) return Modbus::Status_BadIllegalDataAddress;
        m_registers[offset] = value;
        return Modbus::Status_Good;
    }
    
    // Implement other interface methods...
    
private:
    uint16_t m_registers[1000];
};

int main() {
    MyDevice device;
    
    Modbus::TcpSettings settings;
    settings.port    = 502;
    settings.timeout = 5000;
    settings.maxconn = 20; // Support 20 clients
    
    ModbusTcpServer server(&device);
    server.setPort(settings.port);
    server.setTimeout(settings.timeout);
    server.setMaxConnections(settings.maxconn);
    
    printf("Server listening on port %u\n", settings.port);
    
    // Main loop
    bool running = true;
    while (running) {
        server.process(); // Handle all connections
        Modbus::msleep(1);
    }
    
    server.close();
    
    return 0;
}
```

### Example 5: ASCII Serial Server {#example-ascii-server}

```cpp
#include <Modbus.h>
#include <ModbusServerPort.h>

class MyDevice : public ModbusInterface {
    // ... implement interface
};

int main() {
    MyDevice device;
    
    // ASCII protocol configuration (7E1)
    Modbus::SerialSettings settings;
    settings.portName         = "COM1";
    settings.baudRate         = 9600;
    settings.dataBits         = 7;
    settings.parity           = Modbus::EvenParity;
    settings.stopBits         = Modbus::OneStop;
    settings.flowControl      = Modbus::NoFlowControl;
    settings.timeoutFirstByte = 1000;
    settings.timeoutInterByte = 50;
    
    ModbusServerPort *server = Modbus::createServerPort(&device, Modbus::ASC, 
                                                        &settings, false);
    
    printf("ASCII server running on %s\n", settings.portName);
    
    bool running = true;
    while (running) {
        server->process();
        Modbus::msleep(1);
    }
    
    server->close();
    
    delete server;
    return 0;
}
```

### Example 6: Qt-Based Configuration {#example-qt-configuration}

```cpp
#include <ModbusQt.h>
#include <ModbusClient.h>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Configure using Qt settings
    Modbus::Settings settings;
    const Modbus::Strings &s = Modbus::Strings::instance();
    
    settings[s.unit]    = 1;
    settings[s.type]    = s.TCP;
    settings[s.host]    = QString("192.168.1.100");
    settings[s.port]    = 502;
    settings[s.timeout] = 5000;
    settings[s.tries]   = 3;
    
    ModbusClient client;
    client.setSettings(settings);
    
    // Connect signals
    QObject::connect(&client, &ModbusClient::signalOpened, [](const QString &source) {
        qDebug() << "Opened:" << source;
    });
    
    QObject::connect(&client, &ModbusClient::signalTx, [](const QString &source, 
                                                           const QByteArray &data) {
        qDebug() << "Tx:" << data.toHex();
    });
    
    // Read registers
    uint16_t registers[10];
    Modbus::StatusCode status = client.readHoldingRegisters(1, 0, 10, registers);
    
    client.close();
    
    return 0;
}
```

---

## Best Practices {#config-best-practices}

### 1. Always Check Status Codes {#bp-check-status}

```cpp
Modbus::StatusCode status = client->open();
if (Modbus::StatusIsGood(status)) {
    // Proceed with operations
} else if (Modbus::StatusIsBad(status)) {
    // Handle error
    const char *error = client->lastErrorText();
    printf("Error: %s\n", error);
}
```

### 2. Use Appropriate Timeouts {#bp-timeouts}

- **TCP timeout**: 3000-5000 ms for LAN, 10000+ ms for WAN
- **Serial first byte**: 1000-3000 ms for typical devices
- **Serial inter-byte**: 1-50 ms depending on baud rate

### 3. Configure Retry Count {#bp-retry-count}

```cpp
client->setTries(3); // Retry failed operations up to 3 times
```

### 4. Use Non-Blocking Mode for GUI {#bp-nonblocking-gui}

```cpp
ModbusClientPort *client = Modbus::createClientPort(Modbus::TCP, &settings, false);
// Prevents GUI freezing
```

### 5. Clean Resource Management {#bp-resource-management}

```cpp
ModbusClientPort *client = new ModbusClientPort(port);
// ... use client
client->close();
delete client; // Automatically deletes port
```

### 6. Verify Serial Port Names {#bp-serial-port-names}

```cpp
// Windows
settings.portName = "COM3";  // Correct
settings.portName = "\\\\.\\COM10";  // For COM10 and above

// Linux
settings.portName = "/dev/ttyUSB0";  // USB adapter
settings.portName = "/dev/ttyS0";    // Built-in port
```

### 7. Use Defaults Classes {#bp-use-defaults}

```cpp
const ModbusSerialPort::Defaults &defaults = ModbusSerialPort::Defaults::instance();
settings.baudRate = defaults.baudRate; // Use library defaults
```

### 8. Server Connection Limits {#bp-connection-limits}

```cpp
server.setMaxConnections(10); // Limit to prevent resource exhaustion
```

### 9. Enable Broadcast Carefully {#bp-broadcast}

```cpp
// Client
client->setBroadcastEnabled(true); // Only if needed
client->writeSingleRegister(0, 100, 1234); // Broadcast write (no response)

// Server
server->setBroadcastEnabled(true); // Enable if clients send broadcasts
```

### 10. Monitor Statistics {#bp-monitor-statistics}

```cpp
Modbus::StatusCode lastStatus = client->lastStatus();
uint32_t actualTries = client->lastTries();
const char *errorText = client->lastErrorText();
```

---

## See Also {#configuration-see-also}

- \ref client_guide "Client Programming Guide" - Complete client usage examples
- \ref server_guide "Server Programming Guide" - Complete server usage examples
- \ref api_reference "API Reference" - Detailed API documentation
- \ref protocol_details "Protocol Details" - Modbus protocol specifications

---

**Copyright © 2024 Serhii Marchuk**  
**License:** MIT License (see LICENSE file in project root)
