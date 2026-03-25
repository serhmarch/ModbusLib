# Protocol Details {#protocol_details}

[TOC]

## Overview {#protocol-overview}

Modbus is a widely-used communication protocol in industrial automation and control systems. The ModbusLib library supports all `ProtocolType` variants implemented in the current codebase:

- **Modbus TCP/IP**: Ethernet-based protocol for modern networked systems
- **Modbus UDP**: Datagram-based Modbus transport with transaction ID support
- **Modbus RTU**: Binary protocol for serial communications (RS-232, RS-485)
- **Modbus ASCII**: Human-readable protocol for serial communications with debugging advantages
- **Modbus ASCII over TCP / RTU over TCP**: Serial framing carried over TCP transport
- **Modbus ASCII over UDP / RTU over UDP**: Serial framing carried over UDP transport

All variants share the same basic PDU (Protocol Data Unit) structure containing function codes and data, but differ in ADU framing, transport behavior, and error-checking strategy.

---

## Modbus TCP/IP Protocol {#modbus-tcp-ip-protocol}

### Overview {#protocol-tcp-overview}

Modbus TCP encapsulates the standard Modbus PDU within a TCP/IP packet, making it suitable for Ethernet networks. It uses port 502 by default and adds a MBAP (Modbus Application Protocol) header for message routing and integrity.

### MBAP Header Format {#mbap-header-format}

The MBAP header is 7 bytes and prepended to every Modbus TCP message:

| Byte | Field | Description |
|------|-------|-------------|
| 0-1 | Transaction ID | For synchronization between request/response (big-endian) |
| 2-3 | Protocol ID | Always 0x0000 for Modbus (big-endian) |
| 4-5 | Length | Number of following bytes (big-endian) |
| 6 | Unit ID | Slave address (0-247), 0xFF for broadcast |

### Frame Structure {#protocol-tcp-frame}

```
+-------------+--------------+--------+---------+--------------+----------+
| Transaction | Protocol ID  | Length | Unit ID | Function Code|   Data   |
|    ID (2)   |     (2)      |  (2)   |   (1)   |     (1)      |  (N)     |
+-------------+--------------+--------+---------+--------------+----------+
    MBAP Header (7 bytes)              PDU (N+1 bytes)
```

### Example: Read Holding Registers (FC 03) {#example-read-holding-registers-fc-03}

**Request Frame:**
```
Transaction ID: 0x0001
Protocol ID:    0x0000
Length:         0x0006 (6 bytes follow)
Unit ID:        0x01
Function Code:  0x03
Start Address:  0x0000 (register 0)
Quantity:       0x000A (10 registers)

Hex: 00 01 00 00 00 06 01 03 00 00 00 0A
```

**Response Frame:**
```
Transaction ID: 0x0001
Protocol ID:    0x0000
Length:         0x0017 (23 bytes follow)
Unit ID:        0x01
Function Code:  0x03
Byte Count:     0x14 (20 bytes = 10 registers x 2)
Data:           [20 bytes of register values]

Hex: 00 01 00 00 00 17 01 03 14 [20 data bytes]
```

### C++ Implementation Example {#protocol-tcp-impl}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>
#include <ModbusTcpPort.h>

ModbusTcpPort *tcp = new ModbusTcpPort(false);
tcp->setHost("192.168.1.100");
tcp->setPort(502);

ModbusClientPort client(tcp);

uint16_t regs[10] = {0};
Modbus::StatusCode status = client.readHoldingRegisters(1, 0, 10, regs);
while (Modbus::StatusIsProcessing(status))
    status = client.readHoldingRegisters(1, 0, 10, regs);

if (Modbus::StatusIsGood(status)) {
    for (int i = 0; i < 10; ++i)
        std::cout << "Register " << i << ": " << regs[i] << std::endl;
} else {
    std::cout << "Error: " << client.lastErrorText() << std::endl;
}

client.close();
```

### Advantages {#tcp-advantages}

- **No checksum required**: TCP/IP provides error detection at lower layers
- **Fast communication**: No inter-frame delays required
- **Network scalability**: Supports multiple simultaneous connections
- **Remote access**: Works over LANs and WANs
- **Transaction matching**: Transaction ID enables request/response correlation

### Disadvantages {#tcp-disadvantages}

- **Higher overhead**: MBAP header + TCP/IP stack overhead
- **Network dependency**: Requires Ethernet infrastructure
- **Not deterministic**: Subject to network latency and congestion
- **Security concerns**: Requires additional measures (firewalls, VPNs)

---

## Modbus UDP Protocol {#modbus-udp-protocol}

### Overview {#udp-overview}

Modbus UDP uses datagram transport and is implemented by `ModbusUdpPort`. It is useful in closed networks where low-latency request/response is preferred over connection-oriented delivery.

### Transport Characteristics {#udp-transport-characteristics}

- **Connectionless**: No session establishment handshake
- **Datagram-based**: Each request/response is an independent packet
- **No retransmission by transport**: Packet loss/reordering must be handled at application level
- **Transaction ID support**: `ModbusUdpPort` provides transaction ID management APIs (`transactionId()`, `setTransactionId()`, `setNextRequestRepeated()`)

### C++ Implementation Example {#protocol-udp-impl}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>
#include <ModbusUdpPort.h>

ModbusUdpPort *udp = new ModbusUdpPort(false);
udp->setHost("192.168.1.120");
udp->setPort(502);

ModbusClientPort client(udp);

uint16_t regs[4] = {0};
Modbus::StatusCode status = client.readHoldingRegisters(1, 0, 4, regs);
while (Modbus::StatusIsProcessing(status))
    status = client.readHoldingRegisters(1, 0, 4, regs);

if (Modbus::StatusIsBad(status))
    std::cout << "UDP read error: " << client.lastErrorText() << std::endl;
```

### Advantages {#udp-advantages}

- **Low overhead**: No connection state and lightweight transport
- **Simple deployment**: Useful for request/response polling on local networks

### Disadvantages {#udp-disadvantages}

- **No delivery guarantee**: Lost packets require application retries
- **Possible reordering**: Responses may arrive out of sequence on unstable networks
- **Security concerns**: Requires network isolation or additional controls

---

## Modbus RTU Protocol {#modbus-rtu-protocol}

### Overview {#rtu-overview}

Modbus RTU (Remote Terminal Unit) is a compact binary protocol designed for serial communications. It uses binary encoding for efficient data transmission and CRC-16 error checking for data integrity.

### Frame Structure {#rtu-frame-structure}

```
+---------+--------------+----------+-------------+
| Address | Function Code|   Data   |  CRC-16     |
|   (1)   |     (1)      |   (N)    |  (2 bytes)  |
+---------+--------------+----------+-------------+
    ^                                              ^
    Start (3.5 char silence)              End (3.5 char silence)
```

### Timing Requirements {#timing-requirements}

RTU frames are delimited by silent intervals:

- **Frame start**: Minimum 3.5 character times of silence
- **Frame end**: Minimum 3.5 character times of silence
- **Inter-character timeout**: Maximum 1.5 character times between bytes

**Character time calculation:**
```
T_char = (1 start bit + 8 data bits + 1 parity bit + 1 stop bit) / baud_rate
T_char = 11 bits / baud_rate

At 9600 baud: T_char = 11 / 9600 = 1.146 ms
              3.5 x T_char = 4.01 ms
              1.5 x T_char = 1.72 ms

At 19200 baud: T_char = 0.573 ms
               3.5 x T_char = 2.00 ms (minimum 1.75 ms per spec)
```

For baud rates > 19200, use fixed timing:
- Frame delimiter: 1.75 ms
- Inter-character timeout: 0.75 ms

### CRC-16 Calculation {#crc-16-calculation}

RTU uses CRC-16-ANSI (polynomial 0xA001) with initial value 0xFFFF:

```cpp
uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;  // Low byte first in frame
}
```

### Example: Read Coils (FC 01) {#example-read-coils-fc-01}

**Request Frame:**
```
Address:       0x01 (Unit 1)
Function Code: 0x01 (Read Coils)
Start Address: 0x0013 (coil 19)
Quantity:      0x0025 (37 coils)
CRC-16:        0x0E84

Hex: 01 01 00 13 00 25 0E 84
```

**Response Frame:**
```
Address:       0x01
Function Code: 0x01
Byte Count:    0x05 (5 bytes for 37 bits)
Coil Status:   0xCD 0x6B 0xB2 0x0E 0x1B
CRC-16:        0x45E6

Hex: 01 01 05 CD 6B B2 0E 1B 45 E6

Coil status bits (LSB first):
Byte 1 (0xCD = 11001101): Coils 19-26
Byte 2 (0x6B = 01101011): Coils 27-34
...
```

### C++ Implementation Example {#protocol-rtu-impl}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>
#include <ModbusRtuPort.h>

ModbusRtuPort *rtu = new ModbusRtuPort(false);
rtu->setPortName("COM1");
rtu->setBaudRate(9600);
rtu->setDataBits(8);
rtu->setParity(Modbus::EvenParity);
rtu->setStopBits(Modbus::OneStop);

ModbusClientPort client(rtu);

bool coils[100] = {false};
Modbus::StatusCode status = client.readCoilsAsBoolArray(1, 0, 100, coils);
while (Modbus::StatusIsProcessing(status))
    status = client.readCoilsAsBoolArray(1, 0, 100, coils);

if (Modbus::StatusIsGood(status)) {
    for (int i = 0; i < 100; ++i)
        if (coils[i]) std::cout << "Coil " << i << " is ON" << std::endl;
} else {
    std::cout << "Error: " << client.lastErrorText() << std::endl;
}

client.close();
```

### Advantages {#rtu-advantages}

- **Compact format**: Binary encoding minimizes frame size
- **Efficient**: Low overhead suitable for slow serial links
- **Robust error detection**: CRC-16 provides excellent error detection
- **Deterministic**: Predictable timing behavior
- **Widely supported**: Industry standard for serial communications

### Disadvantages {#rtu-disadvantages}

- **Timing critical**: Requires precise inter-frame timing
- **Not human-readable**: Binary format difficult to debug
- **Baud rate sensitive**: Timing calculations depend on baud rate
- **Serial limitations**: Distance and speed limitations of RS-232/RS-485

---

## Modbus ASCII Protocol {#modbus-ascii-protocol}

### Overview {#ascii-overview}

Modbus ASCII encodes all data as ASCII hexadecimal characters, making frames human-readable. Each byte is represented by two ASCII characters (0-9, A-F), doubling the frame size but simplifying debugging and manual testing.

### Frame Structure {#ascii-frame-structure}

```
+-------+---------+--------------+----------+-----+------+
| Start | Address | Function Code|   Data   | LRC | End  |
|  ':'  |  (2)    |     (2)      |   (2N)   | (2) |CR LF |
+-------+---------+--------------+----------+-----+------+
    0x3A    ASCII      ASCII         ASCII    ASCII  0x0D 0x0A
```

- **Start delimiter**: `:` (0x3A)
- **End delimiter**: CR LF (0x0D 0x0A)
- **All data**: ASCII hexadecimal characters (0-9, A-F)

### LRC (Longitudinal Redundancy Check) Calculation {#lrc-longitudinal-redundancy-check-calculation}

The LRC is calculated as the two's complement of the sum of all data bytes:

```cpp
uint8_t calculateLRC(const uint8_t* data, size_t length) {
    uint8_t lrc = 0;
    
    for (size_t i = 0; i < length; ++i) {
        lrc += data[i];
    }
    
    // Two's complement
    return (uint8_t)(-((int8_t)lrc));
}
```

### Example: Write Single Register (FC 06) {#example-write-single-register-fc-06}

**Binary representation:**
```
Address:       0x01
Function Code: 0x06
Register Addr: 0x0001
Value:         0x0003
LRC:           0xF5
```

**ASCII Frame:**
```
Start:  :
Data:   01 06 00 01 00 03 F5
End:    CR LF

Complete frame (ASCII): :0106000100F5\r\n
Hex bytes: 3A 30 31 30 36 30 30 30 31 30 30 30 33 46 35 0D 0A
```

### Example: Read Input Registers (FC 04) {#example-read-input-registers-fc-04}

**Request Frame (ASCII):**
```
:010400000002F8\r\n

Breakdown:
:     - Start
01    - Address (unit 1)
04    - Function code
0000  - Start address (register 0)
0002  - Quantity (2 registers)
F8    - LRC
\r\n  - End
```

**Response Frame (ASCII):**
```
:01040400010002F8\r\n

Breakdown:
:     - Start
01    - Address
04    - Function code
04    - Byte count (4 bytes = 2 registers)
0001  - Register 0 value
0002  - Register 1 value
F8    - LRC
\r\n  - End
```

### C++ Implementation Example {#protocol-ascii-impl}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>
#include <ModbusAscPort.h>

ModbusAscPort *asc = new ModbusAscPort(false);
asc->setPortName("COM2");
asc->setBaudRate(9600);
asc->setDataBits(7);
asc->setParity(Modbus::EvenParity);
asc->setStopBits(Modbus::OneStop);

ModbusClientPort client(asc);

Modbus::StatusCode status = client.writeSingleRegister(1, 100, 0x1234);
while (Modbus::StatusIsProcessing(status))
    status = client.writeSingleRegister(1, 100, 0x1234);

if (Modbus::StatusIsGood(status))
    std::cout << "Register written successfully" << std::endl;
else
    std::cout << "Error: " << client.lastErrorText() << std::endl;

client.close();
```

### Advantages {#ascii-advantages}

- **Human-readable**: Easy to debug with terminal programs
- **Simple parsing**: Standard ASCII characters
- **Flexible timing**: Less timing critical than RTU
- **Easy manual testing**: Can send frames from terminal emulator
- **Noise tolerant**: Start/end delimiters provide clear framing

### Disadvantages {#ascii-disadvantages}

- **Double size**: Each byte becomes two ASCII characters
- **Slower transmission**: Takes twice as long to send
- **Less efficient**: Higher bandwidth usage
- **Weaker error detection**: LRC is less robust than CRC-16
- **Case sensitive**: Upper case hex characters required (A-F, not a-f)

---

## RTU/ASCII over TCP and UDP {#rtu-ascii-over-tcp-udp}

### Overview {#rtu-ascii-over-tcp-udp-overview}

ModbusLib also provides hybrid transports that keep RTU/ASCII framing while using socket transports:

- `ModbusAscOverTcpPort`  (`Modbus::ASCvTCP`)
- `ModbusRtuOverTcpPort`  (`Modbus::RTUvTCP`)
- `ModbusAscOverUdpPort`  (`Modbus::ASCvUDP`)
- `ModbusRtuOverUdpPort`  (`Modbus::RTUvUDP`)

These classes are useful for protocol gateways, compatibility testing, and mixed infrastructures where serial-style framing must be preserved over IP transport.

### C++ Implementation Examples {#rtu-ascii-over-tcp-udp-impl}

```cpp
#include <Modbus.h>
#include <ModbusClientPort.h>
#include <ModbusAscOverTcpPort.h>
#include <ModbusRtuOverUdpPort.h>

// ASCII over TCP
ModbusAscOverTcpPort *ascTcp = new ModbusAscOverTcpPort(false);
ascTcp->setHost("192.168.1.50");
ascTcp->setPort(502);
ModbusClientPort ascTcpClient(ascTcp);

// RTU over UDP
ModbusRtuOverUdpPort *rtuUdp = new ModbusRtuOverUdpPort(false);
rtuUdp->setHost("192.168.1.51");
rtuUdp->setPort(502);
ModbusClientPort rtuUdpClient(rtuUdp);
```

### Practical Notes {#rtu-ascii-over-tcp-udp-notes}

- Choose `ASCvTCP/ASCvUDP` when human-readable ASCII framing is needed over IP.
- Choose `RTUvTCP/RTUvUDP` when RTU-style binary framing compatibility is required.
- Apply the same `StatusCode` handling and retry logic as with standard TCP/UDP client ports.

---

## Protocol Variant Comparison {#protocol-variant-comparison}

| ProtocolType | Main class | Transport | Framing | Frame check | Delivery guarantee | Human-readable |
|--------------|------------|-----------|---------|-------------|--------------------|----------------|
| `Modbus::TCP` | `ModbusTcpPort` | TCP stream | MBAP + PDU | Transport-level checks | Yes (TCP) | No |
| `Modbus::UDP` | `ModbusUdpPort` | UDP datagram | MBAP-like datagram framing + PDU | Transport-level checks | No (app retries recommended) | No |
| `Modbus::RTU` | `ModbusRtuPort` | Serial (RS-232/RS-485) | RTU ADU | CRC-16 | N/A (serial link) | No |
| `Modbus::ASC` | `ModbusAscPort` | Serial (RS-232/RS-485) | ASCII ADU (`:` ... CRLF) | LRC | N/A (serial link) | Yes |
| `Modbus::ASCvTCP` | `ModbusAscOverTcpPort` | TCP stream | ASCII framing over TCP | LRC + TCP reliability | Yes (TCP) | Yes |
| `Modbus::RTUvTCP` | `ModbusRtuOverTcpPort` | TCP stream | RTU framing over TCP | CRC-16 + TCP reliability | Yes (TCP) | No |
| `Modbus::ASCvUDP` | `ModbusAscOverUdpPort` | UDP datagram | ASCII framing over UDP | LRC | No (app retries recommended) | Yes |
| `Modbus::RTUvUDP` | `ModbusRtuOverUdpPort` | UDP datagram | RTU framing over UDP | CRC-16 | No (app retries recommended) | No |

**Selection guidance:**
- Use `TCP` for standard industrial Ethernet interoperability.
- Use `UDP` when low overhead is preferred and your application can tolerate or handle packet loss.
- Use `RTU`/`ASC` for native serial buses.
- Use `RTUvTCP/ASCvTCP/RTUvUDP/ASCvUDP` for gateways and compatibility scenarios where serial framing must be preserved over IP.

---

## Modbus Function Codes {#modbus-function-codes}

### Function Code Overview {#function-code-overview}

| FC | Function Name | Type | Description |
|----|---------------|------|-------------|
| 01 | Read Coils | Bit Read | Read 1-2000 contiguous coils |
| 02 | Read Discrete Inputs | Bit Read | Read 1-2000 contiguous discrete inputs |
| 03 | Read Holding Registers | Word Read | Read 1-125 contiguous registers |
| 04 | Read Input Registers | Word Read | Read 1-125 contiguous input registers |
| 05 | Write Single Coil | Bit Write | Write single coil (ON/OFF) |
| 06 | Write Single Register | Word Write | Write single holding register |
| 07 | Read Exception Status | Special | Read device exception status byte |
| 08 | Diagnostics | Special | Diagnostics subfunctions (serial line diagnostics and counters) |
| 11 | Get Comm Event Counter | Special | Read communication status/event counter |
| 12 | Get Comm Event Log | Special | Read communication event log |
| 15 | Write Multiple Coils | Bit Write | Write multiple coils (1-1968) |
| 16 | Write Multiple Registers | Word Write | Write multiple registers (1-123) |
| 17 | Report Server ID | Special | Read server identification payload |
| 20 | Read File Record | File | Read one or more file records |
| 21 | Write File Record | File | Write one or more file records |
| 22 | Mask Write Register | Word Write | Modify register using AND/OR masks |
| 23 | Read/Write Multiple Registers | Word Read/Write | Combined read/write operation |
| 24 | Read FIFO Queue | Special | Read FIFO queue of register values |
| 43/14 | Read Device Identification | Encapsulated Interface | Read identity objects via MEI type 0x0E |

### FC 01: Read Coils (0x01) {#fc-01-read-coils-0x01}

Reads the ON/OFF status of discrete output coils.

**Request PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) |
|     0x01     |   0x0000     |   0x0001     |
+--------------+--------------+--------------+
```

**Response PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Byte Count(1)| Coil Status  |
|     0x01     |     0x01     |   (N bytes)  |
+--------------+--------------+--------------+
```

**Example:**
```cpp
// Read 20 coils starting at address 100
bool coils[20] = {false};
Modbus::StatusCode status = port.readCoilsAsBoolArray(slaveId, 100, 20, coils);
```

### FC 02: Read Discrete Inputs (0x02) {#fc-02-read-discrete-inputs-0x02}

Reads the ON/OFF status of discrete inputs (read-only).

**Request/Response:** Same format as FC 01

**Example:**
```cpp
// Read 16 discrete inputs starting at address 0
bool inputs[16] = {false};
Modbus::StatusCode status = port.readDiscreteInputsAsBoolArray(slaveId, 0, 16, inputs);
```

### FC 03: Read Holding Registers (0x03) {#fc-03-read-holding-registers-0x03}

Reads 16-bit holding registers (read/write).

**Request PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) |
|     0x03     |   0x0000     |   0x000A     |
+--------------+--------------+--------------+
```

**Response PDU:**
```
+--------------+--------------+----------------------+
| Function (1) | Byte Count(1)| Register Values      |
|     0x03     |     0x14     | (2 bytes per reg)    |
+--------------+--------------+----------------------+
```

**Example:**
```cpp
// Read 10 holding registers starting at address 0
uint16_t values[10] = {0};
Modbus::StatusCode status = port.readHoldingRegisters(slaveId, 0, 10, values);
```

### FC 04: Read Input Registers (0x04) {#fc-04-read-input-registers-0x04}

Reads 16-bit input registers (read-only, typically for sensor data).

**Request/Response:** Same format as FC 03

**Example:**
```cpp
// Read 5 input registers starting at address 10
uint16_t values[5] = {0};
Modbus::StatusCode status = port.readInputRegisters(slaveId, 10, 5, values);
```

### FC 05: Write Single Coil (0x05) {#fc-05-write-single-coil-0x05}

Writes a single coil to ON (0xFF00) or OFF (0x0000).

**Request PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Coil Addr(2) | Value (2)    |
|     0x05     |   0x0000     |   0xFF00     |
+--------------+--------------+--------------+
                 ^
             ON = 0xFF00, OFF = 0x0000
```

**Response:** Echo of request (same format)

**Example:**
```cpp
// Turn ON coil at address 50
Modbus::StatusCode status = port.writeSingleCoil(slaveId, 50, true);

// Turn OFF coil at address 50
status = port.writeSingleCoil(slaveId, 50, false);
```

### FC 06: Write Single Register (0x06) {#fc-06-write-single-register-0x06}

Writes a single 16-bit register.

**Request PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Reg Addr (2) | Value (2)    |
|     0x06     |   0x0001     |   0x0003     |
+--------------+--------------+--------------+
```

**Response:** Echo of request

**Example:**
```cpp
// Write value 1234 to register 100
Modbus::StatusCode status = port.writeSingleRegister(slaveId, 100, 1234);
```

### FC 07: Read Exception Status (0x07) {#fc-07-read-exception-status-0x07}

Reads one-byte exception status from the remote device.

**Request PDU:**
```
+--------------+
| Function (1) |
|     0x07     |
+--------------+
```

**Response PDU:**
```
+--------------+--------------+
| Function (1) | Status (1)   |
|     0x07     | bit flags    |
+--------------+--------------+
```

**Example:**
```cpp
uint8_t exceptionStatus = 0;
Modbus::StatusCode status = port.readExceptionStatus(slaveId, &exceptionStatus);
```

### FC 08: Diagnostics (0x08) {#fc-08-diagnostics-0x08}

In v0.5, diagnostics are exposed as dedicated subfunction methods instead of a generic `diagnostics(...)` entry point.

Common diagnostics methods include:
- `diagnosticsReturnQueryData(...)`
- `diagnosticsRestartCommunicationsOption(...)`
- `diagnosticsReturnDiagnosticRegister(...)`
- `diagnosticsReturnBusMessageCount(...)`
- `diagnosticsReturnBusCommunicationErrorCount(...)`
- `diagnosticsReturnServerMessageCount(...)`
- `diagnosticsClearOverrunCounterAndFlag(...)`

**FC08 Subfunction map (v0.5):**

| Subfunction (hex) | Name | Method |
|-------------------|------|--------|
| `0x00` | Return Query Data | `diagnosticsReturnQueryData(...)` |
| `0x01` | Restart Communications Option | `diagnosticsRestartCommunicationsOption(...)` |
| `0x02` | Return Diagnostic Register | `diagnosticsReturnDiagnosticRegister(...)` |
| `0x03` | Change ASCII Input Delimiter | `diagnosticsChangeAsciiInputDelimiter(...)` |
| `0x04` | Force Listen Only Mode | `diagnosticsForceListenOnlyMode(...)` |
| `0x0A` | Clear Counters and Diagnostic Register | `diagnosticsClearCountersAndDiagnosticRegister(...)` |
| `0x0B` | Return Bus Message Count | `diagnosticsReturnBusMessageCount(...)` |
| `0x0C` | Return Bus Communication Error Count | `diagnosticsReturnBusCommunicationErrorCount(...)` |
| `0x0D` | Return Bus Exception Error Count | `diagnosticsReturnBusExceptionErrorCount(...)` |
| `0x0E` | Return Server Message Count | `diagnosticsReturnServerMessageCount(...)` |
| `0x0F` | Return Server No Response Count | `diagnosticsReturnServerNoResponseCount(...)` |
| `0x10` | Return Server NAK Count | `diagnosticsReturnServerNAKCount(...)` |
| `0x11` | Return Server Busy Count | `diagnosticsReturnServerBusyCount(...)` |
| `0x12` | Return Bus Character Overrun Count | `diagnosticsReturnBusCharacterOverrunCount(...)` |
| `0x14` | Clear Overrun Counter and Flag | `diagnosticsClearOverrunCounterAndFlag(...)` |

**Example:**
```cpp
uint16_t busMessageCount = 0;
Modbus::StatusCode status = port.diagnosticsReturnBusMessageCount(slaveId, &busMessageCount);
```

### FC 11: Get Comm Event Counter (0x0B) {#fc-11-get-comm-event-counter-0x0b}

Reads communication status and event counter.

**Example:**
```cpp
uint16_t commStatus = 0;
uint16_t eventCount = 0;
Modbus::StatusCode status = port.getCommEventCounter(slaveId, &commStatus, &eventCount);
```

### FC 12: Get Comm Event Log (0x0C) {#fc-12-get-comm-event-log-0x0c}

Reads communication event log payload.

**Example:**
```cpp
uint16_t commStatus = 0;
uint16_t eventCount = 0;
uint16_t messageCount = 0;
uint8_t eventLog[64] = {0};
uint8_t eventLogSize = sizeof(eventLog);

Modbus::StatusCode status = port.getCommEventLog(
    slaveId,
    &commStatus,
    &eventCount,
    &messageCount,
    eventLog,
    &eventLogSize
);
```

### FC 15 (0x0F): Write Multiple Coils {#fc-15-0x0f-write-multiple-coils}

Writes multiple contiguous coils.

**Request PDU:**
```
+--------------+--------------+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) | Byte Count(1)| Coil Values  |
|     0x0F     |   0x0013     |   0x000A     |     0x02     |  (N bytes)   |
+--------------+--------------+--------------+--------------+--------------+
```

**Response PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) |
|     0x0F     |   0x0013     |   0x000A     |
+--------------+--------------+--------------+
```

**Example:**
```cpp
// Write 10 coils starting at address 20
bool coils[10] = {false};
coils[0] = true;   // Coil 20 = ON
coils[5] = true;   // Coil 25 = ON
Modbus::StatusCode status = port.writeMultipleCoilsAsBoolArray(slaveId, 20, 10, coils);
```

### FC 16 (0x10): Write Multiple Registers {#fc-16-0x10-write-multiple-registers}

Writes multiple contiguous 16-bit registers.

**Request PDU:**
```
+--------------+--------------+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) | Byte Count(1)| Reg Values   |
|     0x10     |   0x0001     |   0x0002     |     0x04     |  (N bytes)   |
+--------------+--------------+--------------+--------------+--------------+
```

**Response PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Start Addr(2)| Quantity (2) |
|     0x10     |   0x0001     |   0x0002     |
+--------------+--------------+--------------+
```

**Example:**
```cpp
// Write 5 registers starting at address 100
uint16_t values[] = {100, 200, 300, 400, 500};
Modbus::StatusCode status = port.writeMultipleRegisters(slaveId, 100, 5, values);
```

### FC 17: Report Server ID (0x11) {#fc-17-report-server-id-0x11}

Reads server identification payload.

**Example:**
```cpp
uint8_t serverId[255] = {0};
uint8_t serverIdSize = sizeof(serverId);
Modbus::StatusCode status = port.reportServerID(slaveId, serverId, &serverIdSize);
```

### FC 20: Read File Record (0x14) {#fc-20-read-file-record-0x14}

Reads one or more file records.

**Example:**
```cpp
Modbus::FileRecord records[1] = {
    {0x0001, 0x0000, 4} // file=1, record=0, length=4 registers
};

uint8_t outData[128] = {0};
uint8_t outSize = sizeof(outData);
Modbus::StatusCode status = port.readFileRecord(slaveId, records, 1, outData, &outSize);
```

### FC 21: Write File Record (0x15) {#fc-21-write-file-record-0x15}

Writes one or more file records.

**Example:**
```cpp
Modbus::FileRecord records[1] = {
    {0x0001, 0x0000, 2}
};

uint16_t payloadRegs[] = {0x1111, 0x2222};
uint8_t writtenSize = 0;
Modbus::StatusCode status = port.writeFileRecord(slaveId, records, 1, payloadRegs, &writtenSize);
```

### FC 22 (0x16): Mask Write Register {#fc-22-0x16-mask-write-register}

Modifies a register using AND and OR masks.

**Request PDU:**
```
+--------------+--------------+--------------+--------------+
| Function (1) | Reg Addr (2) | AND Mask (2) | OR Mask (2)  |
|     0x16     |   0x0004     |   0x00F2     |   0x0025     |
+--------------+--------------+--------------+--------------+

Result = (Current_Value AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
```

**Response:** Echo of request

**Example:**
```cpp
// Set bits 0, 2, 5 without changing other bits in register 10
Modbus::StatusCode status = port.maskWriteRegister(
    slaveId, 
    10,           // Register address
    0xFFFF,       // AND mask (keep all bits)
    0x0025        // OR mask (set bits 0, 2, 5)
);
```

### FC 23 (0x17): Read/Write Multiple Registers {#fc-23-0x17-read-write-multiple-registers}

Combines read and write operations in a single transaction.

**Request PDU:**
```
+--------------+--------------+--------------+--------------+--------------+
| Function (1) | Read Addr(2) | Read Qty (2) | Write Addr(2)| Write Qty(2) |
|     0x17     |   0x0003     |   0x0006     |   0x000E     |   0x0003     |
+--------------+--------------+--------------+--------------+--------------+
 +-------------+--------------+
 | Byte Count(1)| Write Values |
 |     0x06     |  (N bytes)   |
 +-------------+--------------+
```

**Response PDU:**
```
+--------------+--------------+--------------+
| Function (1) | Byte Count(1)| Read Values  |
|     0x17     |     0x0C     |  (N bytes)   |
+--------------+--------------+--------------+
```

**Example:**
```cpp
// Read 6 registers from address 3, write 3 registers to address 14
uint16_t writeValues[] = {100, 200, 300};
uint16_t readValues[6] = {0};
Modbus::StatusCode status = port.readWriteMultipleRegisters(
    slaveId,
    3,              // Read start address
    6,              // Read quantity
    readValues,     // Read output buffer
    14,             // Write start address
    3,              // Write quantity
    writeValues     // Values to write
);
```

### FC 24: Read FIFO Queue (0x18) {#fc-24-read-fifo-queue-0x18}

Reads values from a FIFO queue in the remote device.

**Example:**
```cpp
uint16_t fifoValues[32] = {0};
uint16_t fifoCount = 0;
Modbus::StatusCode status = port.readFIFOQueue(slaveId, 0, fifoValues, &fifoCount);
```

### FC 43 / MEI 0x0E: Read Device Identification {#fc-43-mei-0x0e-read-device-identification}

Reads identity objects (vendor/product/revision and extended objects).

**Example:**
```cpp
uint8_t idData[256] = {0};
uint8_t idDataSize = sizeof(idData);
uint8_t objectCount = 0;
uint8_t conformityLevel = 0;
bool moreFollows = false;
uint8_t nextObjectId = 0;

Modbus::StatusCode status = port.readDeviceIdentification(
    slaveId,
    1, // Basic device identification
    0, // Start object id
    idData,
    &idDataSize,
    &objectCount,
    &conformityLevel,
    &moreFollows,
    &nextObjectId
);
```

---

## Error Handling {#protocol-error-handling}

### Exception Responses {#exception-responses}

When a Modbus device encounters an error processing a request, it returns an exception response:

**Exception Response Format:**
```
+------------------+--------------+
| Function + 0x80  | Exception    |
|      (1)         |   Code (1)   |
+------------------+--------------+
```

The function code has its MSB set (original + 0x80):
- Request FC 03 → Exception FC 0x83
- Request FC 06 → Exception FC 0x86
- Request FC 16 → Exception FC 0x90

### Exception Codes {#exception-codes}

| Code | Name | Description |
|------|------|-------------|
| 0x01 | Illegal Function | Function code not supported |
| 0x02 | Illegal Data Address | Register/coil address out of range |
| 0x03 | Illegal Data Value | Value in query field is invalid |
| 0x04 | Slave Device Failure | Unrecoverable error occurred |
| 0x05 | Acknowledge | Long operation in progress, client should poll |
| 0x06 | Slave Device Busy | Device busy processing long command |
| 0x08 | Memory Parity Error | Parity error in extended memory |
| 0x0A | Gateway Path Unavailable | Gateway misconfigured or overloaded |
| 0x0B | Gateway Target Failed | Target device failed to respond |

### Example Exception Response {#example-exception-response}

**Request (Read non-existent register):**
```
RTU Hex: 01 03 9C 41 00 02 [CRC]
         |  |  +----+  +--+
         |  |     |      +-- Quantity: 2 registers
         |  |     +-- Address: 40001 (may not exist)
         |  +-- Function: Read Holding Registers
         +-- Unit ID: 1
```

**Exception Response:**
```
RTU Hex: 01 83 02 [CRC]
         |  |  |
         |  |  +-- Exception Code: 0x02 (Illegal Data Address)
         |  +-- Function: 0x83 (0x03 + 0x80)
         +-- Unit ID: 1
```

### C++ Exception Handling {#c-exception-handling}

```cpp
uint16_t values[10] = {0};
Modbus::StatusCode status = port.readHoldingRegisters(slaveId, 9999, 10, values);

if (Modbus::StatusIsBad(status)) {
    switch (status) {
        case Modbus::Status_BadIllegalFunction:
            std::cout << "Function not supported" << std::endl;
            break;
        case Modbus::Status_BadIllegalDataAddress:
            std::cout << "Address out of range" << std::endl;
            break;
        case Modbus::Status_BadIllegalDataValue:
            std::cout << "Invalid data value" << std::endl;
            break;
        case Modbus::Status_BadServerDeviceFailure:
            std::cout << "Device failure" << std::endl;
            break;
        default:
            std::cout << "Error: " << port.lastErrorText() << std::endl;
            break;
    }
}
```

### Protocol Errors {#protocol-errors}

In addition to Modbus exceptions, protocol-level errors can occur:

**Communication Errors:**
- **Timeout**: No response within expected time
- **CRC Error** (RTU): CRC mismatch detected
- **LRC Error** (ASCII): LRC mismatch detected
- **Frame Error**: Invalid frame format or length
- **Overrun**: Buffer overflow or data loss

**Handling Communication Errors:**
```cpp
ModbusRtuPort *rtu = new ModbusRtuPort(false);
rtu->setTimeout(1000);
ModbusClientPort port(rtu);
port.setTries(3);

uint16_t regs[10] = {0};
Modbus::StatusCode status = port.readHoldingRegisters(1, 0, 10, regs);
while (Modbus::StatusIsProcessing(status))
    status = port.readHoldingRegisters(1, 0, 10, regs);

if (Modbus::StatusIsBad(status)) {
    std::cout << "Error status: " << status << std::endl;
    std::cout << "Error text: " << port.lastErrorText() << std::endl;
}
```

---

## Communication Examples {#communication-examples}

### Example 1: TCP Read Holding Registers {#example-1-tcp-read-holding-registers}

**Scenario:** Read 4 holding registers starting at address 0 from unit 17

**Request (TCP):**
```
Hex bytes:
00 01        Transaction ID
00 00        Protocol ID (Modbus)
00 06        Length (6 bytes follow)
11           Unit ID (17)
03           Function Code (Read Holding Registers)
00 00        Starting Address (0)
00 04        Quantity (4 registers)

Complete frame: 00 01 00 00 00 06 11 03 00 00 00 04
```

**Response (TCP):**
```
Hex bytes:
00 01        Transaction ID (matches request)
00 00        Protocol ID
00 0B        Length (11 bytes follow)
11           Unit ID
03           Function Code
08           Byte count (8 bytes = 4 registers)
00 0A        Register 0 = 10
00 14        Register 1 = 20
00 1E        Register 2 = 30
00 28        Register 3 = 40

Complete frame: 00 01 00 00 00 0B 11 03 08 00 0A 00 14 00 1E 00 28
```

### Example 2: RTU Write Multiple Coils {#example-2-rtu-write-multiple-coils}

**Scenario:** Write 8 coils starting at address 100, pattern: 10110100 (0xB4)

**Request (RTU):**
```
Hex bytes:
01           Address (Unit 1)
0F           Function Code (Write Multiple Coils)
00 64        Starting Address (100)
00 08        Quantity (8 coils)
01           Byte count (1 byte for 8 coils)
B4           Coil values (10110100 binary, LSB first)
[CRC-16]     2 bytes CRC

Complete frame: 01 0F 00 64 00 08 01 B4 [CRC_L] [CRC_H]
With CRC: 01 0F 00 64 00 08 01 B4 4D 91
```

**Response (RTU):**
```
Hex bytes:
01           Address
0F           Function Code
00 64        Starting Address
00 08        Quantity written
[CRC-16]     2 bytes CRC

Complete frame: 01 0F 00 64 00 08 [CRC_L] [CRC_H]
With CRC: 01 0F 00 64 00 08 26 4A
```

### Example 3: ASCII Write Single Register {#example-3-ascii-write-single-register}

**Scenario:** Write value 0x1234 to register 50 on unit 5

**Binary representation:**
```
05           Address
06           Function Code
00 32        Register Address (50)
12 34        Value (4660 decimal)
LRC calculation: -(05 + 06 + 00 + 32 + 12 + 34) = -89 = 0xA7
```

**Request (ASCII):**
```
:            Start delimiter
05           Address (ASCII "05")
06           Function code (ASCII "06")
0032         Register address (ASCII "0032")
1234         Value (ASCII "1234")
A7           LRC (ASCII "A7")
\r\n         End delimiter (CR LF)

Complete frame (ASCII characters): :05060032124A7\r\n
Hex bytes: 3A 30 35 30 36 30 30 33 32 31 32 33 34 41 37 0D 0A
```

**Response (ASCII):**
```
Echo of request: :0506003212344A7\r\n
```

### Example 4: Error Response {#example-4-error-response}

**Scenario:** Attempt to read from invalid address 50000

**Request (RTU):**
```
01 03 C3 50 00 01 [CRC]
    |    |     |     |
    |    |     |     +-- CRC-16
    |    |     +-------- Quantity: 1
    |    +-------------- Address: 50000
    +------------------- Read Holding Registers
```

**Exception Response (RTU):**
```
01 83 02 [CRC]
    |  |
    |  +-- Exception: Illegal Data Address (0x02)
    +-- Function + 0x80 = 0x83
```

---

## Best Practices {#protocol-best-practices}

### TCP/IP Protocol {#tcp-ip-protocol}

1. **Connection Management**
   ```cpp
   // One ModbusClientPort per endpoint
   ModbusTcpPort *tcp = new ModbusTcpPort(false);
   tcp->setHost("192.168.1.100");
   tcp->setPort(502);

   ModbusClientPort client(tcp);
   client.setTries(3);
   ```

2. **Transaction Management**
   - Track transaction IDs to match responses to requests
   - Implement timeout handling (typical: 5-10 seconds)
   - Handle simultaneous connections carefully

3. **Network Considerations**
   - Use QoS or VLAN tagging for industrial networks
   - Implement firewall rules to restrict access to port 502
   - Consider ModbusTCP security extensions for critical applications

4. **Error Recovery**
   ```cpp
   // Retry with exponential backoff
   int retries = 0;
   const int maxRetries = 3;
   int delay = 100; // ms
   uint16_t regs[10] = {0};
   
   while (retries < maxRetries) {
       Modbus::StatusCode status = port.readHoldingRegisters(1, 0, 10, regs);
       while (Modbus::StatusIsProcessing(status))
           status = port.readHoldingRegisters(1, 0, 10, regs);

       if (Modbus::StatusIsGood(status)) break;
       
       Modbus::msleep(delay);
       delay *= 2;  // Exponential backoff
       retries++;
   }
   ```

### RTU Protocol {#rtu-protocol}

1. **Serial Port Configuration**
   ```cpp
   // Recommended settings for industrial environments
   port.setBaudRate(9600);      // or 19200 for faster comms
   port.setDataBits(8);
   port.setParity(Modbus::EvenParity);  // Even parity for error detection
    port.setStopBits(Modbus::OneStop);
   port.setFlowControl(Modbus::NoFlowControl);
   ```

2. **Timing Requirements**
   - Respect 3.5 character time silence between frames
   - Calculate timing based on baud rate (or use 1.75ms for >19200)
   - Implement proper timeout: minimum 1 second, adjust based on slave response time

3. **CRC Validation**
   - Always validate CRC on received frames
   - Discard frames with CRC errors immediately
   - Log CRC errors for troubleshooting

4. **Bus Contention**
   ```cpp
   // Implement master arbitration for multi-master systems
   std::mutex busMutex;
   
   void sendRequest() {
       std::lock_guard<std::mutex> locker(busMutex);
       // Only one master can transmit at a time
       uint8_t coils[13] = {0}; // 100 bits
       Modbus::StatusCode status = port.readCoils(1, 0, 100, coils);
   }
   ```

5. **Cable and Termination**
   - Use twisted pair cable for RS-485
   - Add 120 Ohm termination resistors at both ends of RS-485 bus
   - Keep cable length < 1000m for 9600 baud
   - Use shielded cable in noisy environments

### ASCII Protocol {#ascii-protocol}

1. **When to Use ASCII**
   - Debugging and development
   - Manual testing with terminal emulators
   - Environments with high electrical noise (ASCII more tolerant)
   - Educational purposes

2. **Character Handling**
   - Always use uppercase hex characters (A-F, not a-f)
   - Properly handle CR LF end delimiters
   - Implement timeout on incomplete frames

3. **Terminal Testing**
   ```
   # Example terminal session (115200 baud, 8N1)
   
   Send: :010300000001F9\r\n
   Recv: :0103020005F7\r\n
   
   Interpretation:
   Request: Read 1 register from address 0, unit 1
   Response: Value = 0x0005 (5 decimal)
   ```

4. **LRC Validation**
   ```cpp
   // Always validate LRC
   bool validateFrame(const std::string& frame) {
       if (frame.empty() || frame[0] != ':') return false;
       if (frame.size() < 11) return false;  // Minimum frame size
       
       // Extract LRC and data
       std::string dataHex = frame.substr(1, frame.size() - 5);
       std::string lrcHex = frame.substr(frame.size() - 4, 2);

       std::vector<uint8_t> dataBytes(dataHex.size() / 2);
       uint32_t byteCount = Modbus::asciiToBytes(
           reinterpret_cast<const uint8_t*>(dataHex.data()),
           dataBytes.data(),
           static_cast<uint32_t>(dataHex.size())
       );

       uint8_t expectedLRC = calculateLRC(dataBytes.data(), byteCount);
       uint8_t receivedLRC = static_cast<uint8_t>(std::stoul(lrcHex, nullptr, 16));
       
       return expectedLRC == receivedLRC;
   }
   ```

### General Best Practices {#general-best-practices}

1. **Address Planning**
   - Document your register/coil mapping
   - Use consistent address schemes across devices
   - Reserve address ranges for future expansion

2. **Polling Strategy**
   ```cpp
   // Efficient polling with prioritization
   struct PollItem {
       int slaveId;
       int address;
       int count;
       int priority;  // 1=high, 5=low
       int interval;  // ms
       Modbus::Timer lastPoll;
   };
   
   void pollDevices(std::vector<PollItem>& items) {
       Modbus::Timer now = Modbus::timer();
       
       // Sort by priority and interval
       std::sort(items.begin(), items.end(), [now](const PollItem& a, const PollItem& b) {
           int32_t aDue = static_cast<int32_t>(a.lastPoll + a.interval - now);
           int32_t bDue = static_cast<int32_t>(b.lastPoll + b.interval - now);
           if (aDue == bDue) return a.priority < b.priority;
           return aDue < bDue;
       });
       
       // Poll due items
       for (auto& item : items) {
           if (now - item.lastPoll >= item.interval) {
               pollItem(item);
               item.lastPoll = now;
           }
       }
   }
   ```

3. **Data Type Handling**
   ```cpp
   // 32-bit float from two registers (ABCD byte order)
   float registersToFloat(uint16_t reg1, uint16_t reg2) {
       uint32_t combined = ((uint32_t)reg1 << 16) | reg2;
       return *reinterpret_cast<float*>(&combined);
   }
   
   // 32-bit integer from two registers
   int32_t registersToInt32(uint16_t high, uint16_t low) {
       return ((int32_t)high << 16) | low;
   }
   ```

4. **Logging and Diagnostics**
   ```cpp
   void onTx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
   {
       std::cout << source << " TX: " << Modbus::bytesToString(buff, size) << std::endl;
   }

   void onRx(const Modbus::Char *source, const uint8_t *buff, uint16_t size)
   {
       std::cout << source << " RX: " << Modbus::bytesToString(buff, size) << std::endl;
   }

   port.connect(&ModbusClientPort::signalTx, onTx);
   port.connect(&ModbusClientPort::signalRx, onRx);
   ```

5. **Thread Safety**
   ```cpp
   // Use separate port instances per thread
   // or protect with mutex
   class ThreadSafeModbusPort {
       std::mutex mutex;
       ModbusTcpPort *tcp;
       ModbusClientPort client;
       
   public:
       ThreadSafeModbusPort() : tcp(new ModbusTcpPort(false)), client(tcp) {}
       ~ThreadSafeModbusPort() { delete tcp; }

       Modbus::StatusCode readRegisters(int addr, int count, uint16_t *values) {
           std::lock_guard<std::mutex> locker(mutex);
           return client.readHoldingRegisters(1, static_cast<uint16_t>(addr), static_cast<uint16_t>(count), values);
       }
   };
   ```

6. **Performance Optimization**
   - Batch related register reads into single requests
   - Use FC 23 (Read/Write Multiple) to combine operations
   - Minimize polling frequency for non-critical data
   - Cache frequently read values with appropriate timeouts

---

## Summary {#protocol-summary}

This document provides protocol-level details for implementing
all currently supported ModbusLib protocol variants.
Key takeaways:

- **TCP** is best for modern Ethernet networks with minimal overhead
- **UDP** is lightweight but requires application-level reliability handling
- **RTU** provides efficient binary communication over serial links
- **ASCII** offers human-readable debugging for serial applications
- **ASCvTCP/RTUvTCP/ASCvUDP/RTUvUDP** provide serial-style framing over IP transports for compatibility and gateway scenarios

Each protocol has specific timing, framing, and error checking requirements
that must be followed for reliable communication.
The ModbusLib library abstracts these details while providing full control when needed.

For implementation details and API reference, see:
- \ref client_guide "Client Implementation Guide"
- \ref server_guide "Server Implementation Guide"  
- \ref api_reference "API Reference"
