# ModbusLib Architecture {#architecture}

## Overview {#architecture-overview}

ModbusLib is a comprehensive Modbus library for C++ that implements
client and server functionality for TCP, RTU, and ASCII protocols.
The library is structured following object-oriented principles with
clear separation of concerns and supports both Qt and non-Qt environments.

## Core Design Principles {#core-design-principles}

### 1. Protocol Abstraction {#protocol-abstraction}

All protocol implementations (TCP, RTU, ASCII) share a common interface through abstract base classes:

* `ModbusPort` - Abstract interface for low-level communication
* `ModbusClientPort` - Abstract interface for client-side operations  
* `ModbusServerPort` - Abstract interface for server-side operations

### 2. Interface-Based Design {#interface-based-design}

* `ModbusInterface` - Defines all supported Modbus functions (FC 01-24)
* Client implementations delegate to this interface
* Server implementations accept objects implementing this interface

### 3. Blocking and Non-Blocking Modes {#blocking-nonblocking-modes}

* All operations support both blocking and non-blocking modes
* Blocking mode: Waits for completion, returns result or error status
* Non-blocking mode: Returns `Status_Processing` if operation incomplete, requires polling

### 4. Platform Independence {#platform-independence}

* Core functionality works with or without Qt
* Platform-specific implementations for Windows and Unix/Linux
* Optional Qt integration through `ModbusQt.h`

## Architecture Layers {#architecture-layers}

```
+-------------------------------------------------+
|         Application Layer                       |
|  (User code, examples, applications)            |
+--------------+----------------------------------+
               |
+--------------v----------------------------------+
|         Client/Server Layer                     |
|  ModbusClient, ModbusClientPort                 |
|  ModbusTcpServer, ModbusServerResource          |
+--------------+----------------------------------+
               |
+--------------v----------------------------------+
|         Protocol Layer                          |
|  ModbusTcpPort, ModbusRtuPort, ModbusAscPort    |
|  Handles framing, checksums, serialization      |
+--------------+----------------------------------+
               |
+--------------v----------------------------------+
|         Transport Layer                         |
|  Socket (TCP), Serial Port (RTU/ASCII)          |
|  Low-level I/O operations                       |
+-------------------------------------------------+
```

## Core Components {#core-components}

### Port Classes {#port-classes}

#### Base Classes {#base-classes}

* **ModbusPort** - Abstract base class for all port types
  * Handles connection state management
  * Defines protocol-independent interface
  * Supports blocking/non-blocking modes

* **ModbusClientPort** - Base client port implementation
  * Implements `ModbusInterface`
  * Manages request/response state machine
  * Handles error recovery and timeouts
  * Provides signal callbacks (Qt style, but can be used without Qt)
  * Supports resource sharing between multiple clients

* **ModbusServerPort** - Base server port implementation
  * Accepts `ModbusInterface` for request processing
  * Manages incoming connections
  * Handles protocol parsing and response generation

#### Protocol-Specific Implementations {#protocol-specific-implementations}

**TCP Protocol:**
* `ModbusTcpPort` - Client TCP port
* `ModbusTcpServer` - Multi-connection TCP server
* Uses socket I/O for communication

**Serial Protocol (RTU/ASCII):**
* `ModbusSerialPort` - Base serial port class
* `ModbusRtuPort` - RTU protocol implementation
* `ModbusAscPort` - ASCII protocol implementation
* Platform-specific serial I/O

### Client Classes {#client-classes}

#### ModbusClient {#modbusclient-arch}

* Wrapper around `ModbusClientPort`
* Stores unit identifier (slave address)
* Simplifies API by eliminating unit parameter
* Enables multiple clients on single port

```cpp
// Example: Multiple clients sharing single port
ModbusTcpPort tcpPort;
ModbusClientPort port(&tcpPort);
ModbusClient client1(1, &port);
ModbusClient client2(2, &port);
```

#### ModbusClientPort {#modbusclientport-arch}

* Implements Modbus client functionality
* Manages request/response lifecycle
* Supports all Modbus function codes via `ModbusInterface`
* Handles blocking and non-blocking modes
* Internally controls inner `ModbusPort` object in protocol-independent manner

### Server Classes {#server-classes-arch}

#### ModbusServerPort {#modbusserverport-arch}

* Base server port implementation
* Manages incoming requests
* Delegates processing to `ModbusInterface` implementation

#### ModbusTcpServer {#modbustcpserver-arch}

* Manages multiple concurrent TCP connections
* Inherits from `ModbusServerPort`
* Handles client acceptance and routing
* Per-connection state management
* Working in non-blocking mode only in single-threaded mode

#### ModbusServerResource {#modbusserverresource-arch}

* Generic server resource for RTU/ASCII
* Inherits from `ModbusServerPort`
* Manages serial port communication
* Manages single TCP connection

### Interface Classes {#interface-classes}

#### ModbusInterface {#modbusinterface-arch}

* Abstract base defining all supported Modbus functions
* Default implementations return error status
* User implementations handle actual data processing

Methods include:
* Read functions: `readCoils`, `readDiscreteInputs`, `readHoldingRegisters`, `readInputRegisters`
* Write functions: `writeSingleCoil`, `writeSingleRegister`, `writeMultipleCoils`, `writeMultipleRegisters`
* Advanced: `diagnostics`, `maskWriteRegister`, `readWriteMultipleRegisters`, `readFIFOQueue`

## Data Flow {#data-flow}

### Client Request Flow (Synchronous) {#client-request-flow}

```
User Code
    v
ModbusClient / ModbusClientPort (blocking mode)
    v
Protocol Layer (ModbusTcpPort / ModbusRtuPort / ModbusAscPort)
    v
Frame construction (serialization)
    v
CRC/LRC calculation
    v
Transport Layer (Socket / Serial)
    v
[Network / Serial Line]
    v
Server receives request
    v
Response generation
    v
Frame transmission back to client
    v
Client receives response
    v
Response validation (CRC/LRC)
    v
Data extraction and return to user
```

### Server Request Processing Flow {#server-request-flow}

```
Accept Connection
    v
Read incoming data
    v
Parse frame (CRC/LRC validation)
    v
Extract function code and parameters
    v
Call ModbusInterface method
    v
Generate response
    v
Frame construction
    v
CRC/LRC calculation
    v
Send response
    v
Close or await next request
```

## State Machine Design {#state-machine-design}

### Client Port State Machine {#client-state-machine}

The client port operates through distinct states:

* **Idle** - Port is open, no operation in progress
* **Waiting** - Request sent, awaiting server response
* **Processing** - Response received, validating and extracting data
* **Error** - Error occurred, error state active until cleared
* **Closed** - Port is closed, no operations allowed

Transitions occur based on operation calls, timeouts, and response completion. In blocking mode, the state machine advances automatically until completion. In non-blocking mode, the application checks state between poll cycles.

### Server Port State Machine {#server-port-state-machine}

The server port manages connection and request handling:

* **Listening** - Server accepting incoming connections
* **Connected** - Client connection established
* **Receiving** - Reading incoming request data
* **Processing** - Parsing request and executing interface method
* **Transmitting** - Sending response back to client
* **Closed** - Connection or server closed

For TCP servers, each connection maintains independent state. For serial resources, a single connection state is managed. State transitions are event-driven by socket/serial I/O completion and timeout conditions.

## Signal/Callback Mechanism {#signal-callback-mechanism}

ModbusLib implements a signal/slot system for event callbacks (Qt-style but can be used without Qt).

**Port Signals:**
* `signalOpened` - Emitted when port opens
* `signalClosed` - Emitted when port closes
* `signalTx` - Emitted before transmitting data
* `signalRx` - Emitted after receiving data
* `signalError` - Emitted on error

**Callback Signature:**
```cpp
void callback(const Modbus::Char *source, const uint8_t* buff, uint16_t size);
void errorCallback(const Modbus::Char *source, Modbus::StatusCode code, const Modbus::Char *text);
```

## Error Handling {#error-handling}

### Status Codes {#status-codes}

`Modbus::StatusCode` enum provides detailed status information:

* `Status_Good` - Successful operation
* `Status_Processing` - Operation in progress (non-blocking mode)
* `Status_Bad*` - Various error conditions
* Standard Modbus exceptions (0x01-0x0B)
* Protocol-specific errors (TCP, RTU, ASCII)
* Common errors (timeout, buffer overflow, etc.)

## Module Organization {#module-organization}

```
ModbusLib/
+- src/
|   +-- Modbus.h/cpp              # Core definitions, status codes
|   +-- ModbusGlobal.h            # Global definitions and macros
|   +-- ModbusQt.h/cpp            # Additional functionality for Qt integration (optional)
|   +-- ModbusObject.h/cpp        # Base object class
|   +-- ModbusPort.h/cpp          # ModbusPort abstract base
|   +-- ModbusClientPort.h/cpp    # ModbusClientPort implementation
|   +-- ModbusClient.h/cpp        # ModbusClient wrapper
|   +-- ModbusTcpPort.h/cpp       # TCP protocol implementation
|   +-- ModbusSerialPort.h/cpp    # Serial port base class
|   +-- ModbusRtuPort.h/cpp       # RTU protocol implementation
|   +-- ModbusAscPort.h/cpp       # ASCII protocol implementation
|   +-- ModbusServerPort.h/cpp    # ModbusServerPort base
|   +-- ModbusServerResource.h/cpp # Server port implementation for RTU/ASCII or single TCP connection
|   +-- ModbusTcpServer.h/cpp     # TCP server implementation
|   +-- win/                      # Windows-specific implementations
|   \-- unix/                     # Unix/Linux-specific implementations
+-- examples/
|   +-- client/                   # Client examples
|   \-- server/                   # Server examples
\-- tests/                        # Unit tests (GoogleTest)
```

## Key Design Patterns {#key-design-patterns}

### 1. State Machine Pattern {#1-state-machine-pattern}

All port implementations use state machines for managing connection and operation lifecycle.

### 2. Factory Pattern {#2-factory-pattern}

The library provides factory-like creation through constructors and configuration methods.

### 3. Template Method Pattern {#3-template-method-pattern}

Base classes define operation flow, subclasses implement protocol-specific details.

### 4. Observer Pattern {#4-observer-pattern}

Signal/slot mechanism provides event notification.

### 5. Wrapper Pattern {#5-wrapper-pattern}

* `ModbusClient` wraps `ModbusClientPort`
* Simplifies API for single-device communication

## Performance Considerations {#performance-considerations}

### Blocking Mode {#blocking-mode}

* Suitable for single-threaded applications
* Entire thread blocks until operation completes
* Simple to implement and debug

### Non-Blocking Mode {#non-blocking-mode}

* Polling-based operation checking
* Application maintains control loop
* Higher CPU usage due to polling
* Suitable for embedded systems and event-driven architectures

## Thread Safety {#thread-safety}

ModbusLib threading considerations:

* Each port instance should be used by a single thread
* Multiple threads require separate port instances
* Resource sharing between ModbusClient instances is single thread when accessing the same ModbusClientPort

## Summary {#summary}

The ModbusLib architecture provides:

* **Flexibility** through multiple abstraction layers
* **Extensibility** via inheritance and polymorphism
* **Compatibility** with blocking and non-blocking modes
* **Maintainability** through clear separation of concerns
* **Reliability** with comprehensive error handling
* **Portability** across Windows and Unix/Linux platforms
* **Optional Qt integration** for enhanced functionality
