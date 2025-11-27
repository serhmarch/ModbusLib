# Protocol Details {#protocol_details}

[TOC]

## Overview {#protocol-overview}

Modbus is a widely-used communication protocol in industrial automation and control systems. The ModbusLib library supports three major protocol variants, each optimized for different communication media and requirements:

- **Modbus TCP/IP**: Ethernet-based protocol for modern networked systems
- **Modbus RTU**: Binary protocol for serial communications (RS-232, RS-485)
- **Modbus ASCII**: Human-readable protocol for serial communications with debugging advantages

All three variants share the same basic PDU (Protocol Data Unit) structure containing function codes and data, but differ in their ADU (Application Data Unit) framing and error checking mechanisms.

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
┌─────────────┬──────────────┬────────┬─────────┬──────────────┬──────────┐
│Transaction  │ Protocol ID  │ Length │ Unit ID │ Function Code│   Data   │
│    ID (2)   │     (2)      │  (2)   │   (1)   │     (1)      │  (N)     │
└─────────────┴──────────────┴────────┴─────────┴──────────────┴──────────┘
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
Byte Count:     0x14 (20 bytes = 10 registers × 2)
Data:           [20 bytes of register values]

Hex: 00 01 00 00 00 17 01 03 14 [20 data bytes]
```

### C++ Implementation Example {#protocol-tcp-impl}

```cpp
#include <Modbus.h>
#include <ModbusTcpPort.h>

// Create TCP client
Modbus::TcpPort client;
client.setHostName("192.168.1.100");
client.setPort(502);

if (client.open()) {
    // Read 10 holding registers starting at address 0
    Modbus::Response response = client.readHoldingRegisters(
        1,      // Unit ID
        0,      // Start address
        10      // Number of registers
    );
    
    if (response.isValid()) {
        const QVector<uint16_t>& values = response.values();
        for (int i = 0; i < values.size(); ++i) {
            qDebug() << "Register" << i << ":" << values[i];
        }
    }
    client.close();
}
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

## Modbus RTU Protocol {#modbus-rtu-protocol}

### Overview {#rtu-overview}

Modbus RTU (Remote Terminal Unit) is a compact binary protocol designed for serial communications. It uses binary encoding for efficient data transmission and CRC-16 error checking for data integrity.

### Frame Structure {#rtu-frame-structure}

```
┌─────────┬──────────────┬──────────┬─────────────┐
│ Address │ Function Code│   Data   │  CRC-16     │
│   (1)   │     (1)      │   (N)    │  (2 bytes)  │
└─────────┴──────────────┴──────────┴─────────────┘
  ↑                                              ↑
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
              3.5 × T_char = 4.01 ms
              1.5 × T_char = 1.72 ms

At 19200 baud: T_char = 0.573 ms
               3.5 × T_char = 2.00 ms (minimum 1.75 ms per spec)
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
#include <ModbusRtuPort.h>

// Create RTU client on COM1
Modbus::RtuPort client;
client.setPortName("COM1");
client.setBaudRate(9600);
client.setDataBits(8);
client.setParity(Modbus::EvenParity);
client.setStopBits(1);

if (client.open()) {
    // Read 100 coils starting at address 0
    Modbus::Response response = client.readCoils(
        1,      // Slave address
        0,      // Start address
        100     // Number of coils
    );
    
    if (response.isValid()) {
        const QBitArray& coils = response.coils();
        for (int i = 0; i < coils.size(); ++i) {
            if (coils.testBit(i)) {
                qDebug() << "Coil" << i << "is ON";
            }
        }
    }
    client.close();
}
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
┌───────┬─────────┬──────────────┬──────────┬─────┬──────┐
│ Start │ Address │ Function Code│   Data   │ LRC │ End  │
│  ':'  │  (2)    │     (2)      │   (2N)   │ (2) │CR LF │
└───────┴─────────┴──────────────┴──────────┴─────┴──────┘
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
#include <ModbusAscPort.h>

// Create ASCII client
Modbus::AscPort client;
client.setPortName("COM2");
client.setBaudRate(9600);
client.setDataBits(7);  // Typically 7 bits for ASCII
client.setParity(Modbus::EvenParity);
client.setStopBits(1);

if (client.open()) {
    // Write single register
    Modbus::Response response = client.writeSingleRegister(
        1,      // Slave address
        100,    // Register address
        0x1234  // Value
    );
    
    if (response.isValid()) {
        qDebug() << "Register written successfully";
    }
    client.close();
}
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
| 15 | Write Multiple Coils | Bit Write | Write multiple coils (1-1968) |
| 16 | Write Multiple Registers | Word Write | Write multiple registers (1-123) |
| 22 | Mask Write Register | Word Write | Modify register using AND/OR masks |
| 23 | Read/Write Multiple Registers | Word Read/Write | Combined read/write operation |

### FC 01: Read Coils (0x01) {#fc-01-read-coils-0x01}

Reads the ON/OFF status of discrete output coils.

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │
│     0x01     │   0x0000     │   0x0001     │
└──────────────┴──────────────┴──────────────┘
```

**Response PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Byte Count(1)│ Coil Status  │
│     0x01     │     0x01     │   (N bytes)  │
└──────────────┴──────────────┴──────────────┘
```

**Example:**
```cpp
// Read 20 coils starting at address 100
Modbus::Response resp = port.readCoils(slaveId, 100, 20);
if (resp.isValid()) {
    QBitArray coils = resp.coils();
    // coils[0] = coil 100, coils[1] = coil 101, etc.
}
```

### FC 02: Read Discrete Inputs (0x02) {#fc-02-read-discrete-inputs-0x02}

Reads the ON/OFF status of discrete inputs (read-only).

**Request/Response:** Same format as FC 01

**Example:**
```cpp
// Read 16 discrete inputs starting at address 0
Modbus::Response resp = port.readDiscreteInputs(slaveId, 0, 16);
```

### FC 03: Read Holding Registers (0x03) {#fc-03-read-holding-registers-0x03}

Reads 16-bit holding registers (read/write).

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │
│     0x03     │   0x0000     │   0x000A     │
└──────────────┴──────────────┴──────────────┘
```

**Response PDU:**
```
┌──────────────┬──────────────┬──────────────────────┐
│ Function (1) │ Byte Count(1)│ Register Values      │
│     0x03     │     0x14     │ (2 bytes per reg)    │
└──────────────┴──────────────┴──────────────────────┘
```

**Example:**
```cpp
// Read 10 holding registers starting at address 0
Modbus::Response resp = port.readHoldingRegisters(slaveId, 0, 10);
if (resp.isValid()) {
    QVector<uint16_t> values = resp.values();
    qDebug() << "First register:" << values[0];
}
```

### FC 04: Read Input Registers (0x04) {#fc-04-read-input-registers-0x04}

Reads 16-bit input registers (read-only, typically for sensor data).

**Request/Response:** Same format as FC 03

**Example:**
```cpp
// Read 5 input registers starting at address 10
Modbus::Response resp = port.readInputRegisters(slaveId, 10, 5);
```

### FC 05: Write Single Coil (0x05) {#fc-05-write-single-coil-0x05}

Writes a single coil to ON (0xFF00) or OFF (0x0000).

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Coil Addr(2) │ Value (2)    │
│     0x05     │   0x0000     │   0xFF00     │
└──────────────┴──────────────┴──────────────┘
                                 ↑
                         ON = 0xFF00, OFF = 0x0000
```

**Response:** Echo of request (same format)

**Example:**
```cpp
// Turn ON coil at address 50
Modbus::Response resp = port.writeSingleCoil(slaveId, 50, true);

// Turn OFF coil at address 50
resp = port.writeSingleCoil(slaveId, 50, false);
```

### FC 06: Write Single Register (0x06) {#fc-06-write-single-register-0x06}

Writes a single 16-bit register.

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Reg Addr (2) │ Value (2)    │
│     0x06     │   0x0001     │   0x0003     │
└──────────────┴──────────────┴──────────────┘
```

**Response:** Echo of request

**Example:**
```cpp
// Write value 1234 to register 100
Modbus::Response resp = port.writeSingleRegister(slaveId, 100, 1234);
```

### FC 15 (0x0F): Write Multiple Coils {#fc-15-0x0f-write-multiple-coils}

Writes multiple contiguous coils.

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │ Byte Count(1)│ Coil Values  │
│     0x0F     │   0x0013     │   0x000A     │     0x02     │  (N bytes)   │
└──────────────┴──────────────┴──────────────┴──────────────┴──────────────┘
```

**Response PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │
│     0x0F     │   0x0013     │   0x000A     │
└──────────────┴──────────────┴──────────────┘
```

**Example:**
```cpp
// Write 10 coils starting at address 20
QBitArray coils(10);
coils.setBit(0, true);   // Coil 20 = ON
coils.setBit(5, true);   // Coil 25 = ON
// Others default to OFF
Modbus::Response resp = port.writeMultipleCoils(slaveId, 20, coils);
```

### FC 16 (0x10): Write Multiple Registers {#fc-16-0x10-write-multiple-registers}

Writes multiple contiguous 16-bit registers.

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │ Byte Count(1)│ Reg Values   │
│     0x10     │   0x0001     │   0x0002     │     0x04     │  (N bytes)   │
└──────────────┴──────────────┴──────────────┴──────────────┴──────────────┘
```

**Response PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Start Addr(2)│ Quantity (2) │
│     0x10     │   0x0001     │   0x0002     │
└──────────────┴──────────────┴──────────────┘
```

**Example:**
```cpp
// Write 5 registers starting at address 100
QVector<uint16_t> values = {100, 200, 300, 400, 500};
Modbus::Response resp = port.writeMultipleRegisters(slaveId, 100, values);
```

### FC 22 (0x16): Mask Write Register {#fc-22-0x16-mask-write-register}

Modifies a register using AND and OR masks.

**Request PDU:**
```
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ Function (1) │ Reg Addr (2) │ AND Mask (2) │ OR Mask (2)  │
│     0x16     │   0x0004     │   0x00F2     │   0x0025     │
└──────────────┴──────────────┴──────────────┴──────────────┘

Result = (Current_Value AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
```

**Response:** Echo of request

**Example:**
```cpp
// Set bits 0, 2, 5 without changing other bits in register 10
Modbus::Response resp = port.maskWriteRegister(
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
┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┬
│ Function (1) │ Read Addr(2) │ Read Qty (2) │ Write Addr(2)│ Write Qty(2) │
│     0x17     │   0x0003     │   0x0006     │   0x000E     │   0x0003     │
└──────────────┴──────────────┴──────────────┴──────────────┴──────────────┴
 ──────────────┬──────────────┐
  Byte Count(1)│ Write Values │
      0x06     │  (N bytes)   │
 ──────────────┴──────────────┘
```

**Response PDU:**
```
┌──────────────┬──────────────┬──────────────┐
│ Function (1) │ Byte Count(1)│ Read Values  │
│     0x17     │     0x0C     │  (N bytes)   │
└──────────────┴──────────────┴──────────────┘
```

**Example:**
```cpp
// Read 6 registers from address 3, write 3 registers to address 14
QVector<uint16_t> writeValues = {100, 200, 300};
Modbus::Response resp = port.readWriteMultipleRegisters(
    slaveId,
    3,              // Read start address
    6,              // Read quantity
    14,             // Write start address
    writeValues     // Values to write
);

if (resp.isValid()) {
    QVector<uint16_t> readValues = resp.values();
    // Process read values
}
```

---

## Error Handling {#protocol-error-handling}

### Exception Responses {#exception-responses}

When a Modbus device encounters an error processing a request, it returns an exception response:

**Exception Response Format:**
```
┌──────────────────┬──────────────┐
│ Function + 0x80  │ Exception    │
│      (1)         │   Code (1)   │
└──────────────────┴──────────────┘
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
         │  │  └────┘  └──┘
         │  │     │      └── Quantity: 2 registers
         │  │     └── Address: 40001 (may not exist)
         │  └── Function: Read Holding Registers
         └── Unit ID: 1
```

**Exception Response:**
```
RTU Hex: 01 83 02 [CRC]
         │  │  │
         │  │  └── Exception Code: 0x02 (Illegal Data Address)
         │  └── Function: 0x83 (0x03 + 0x80)
         └── Unit ID: 1
```

### C++ Exception Handling {#c-exception-handling}

```cpp
Modbus::Response response = port.readHoldingRegisters(slaveId, 9999, 10);

if (!response.isValid()) {
    if (response.isException()) {
        uint8_t exceptionCode = response.exceptionCode();
        
        switch (exceptionCode) {
            case Modbus::IllegalFunction:
                qDebug() << "Function not supported";
                break;
            case Modbus::IllegalDataAddress:
                qDebug() << "Address out of range";
                break;
            case Modbus::IllegalDataValue:
                qDebug() << "Invalid data value";
                break;
            case Modbus::SlaveDeviceFailure:
                qDebug() << "Device failure";
                break;
            default:
                qDebug() << "Exception code:" << exceptionCode;
        }
    } else {
        qDebug() << "Communication error:" << response.errorString();
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
Modbus::RtuPort port;
port.setTimeout(1000);  // 1 second timeout
port.setRetries(3);     // Retry up to 3 times

Modbus::Response resp = port.readHoldingRegisters(1, 0, 10);

if (!resp.isValid()) {
    Modbus::ErrorCode error = resp.errorCode();
    
    switch (error) {
        case Modbus::TimeoutError:
            qDebug() << "No response from device";
            break;
        case Modbus::CrcError:
            qDebug() << "CRC check failed";
            break;
        case Modbus::FramingError:
            qDebug() << "Invalid frame format";
            break;
        case Modbus::SerialError:
            qDebug() << "Serial port error";
            break;
        default:
            qDebug() << "Error:" << resp.errorString();
    }
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
   │  └────┘  └──┘
   │     │      └── Quantity: 1
   │     └── Address: 50000
   └── Read Holding Registers
```

**Exception Response (RTU):**
```
01 83 02 [CRC]
   │  │
   │  └── Exception: Illegal Data Address (0x02)
   └── Function + 0x80 = 0x83
```

---

## Best Practices {#protocol-best-practices}

### TCP/IP Protocol {#tcp-ip-protocol}

1. **Connection Management**
   ```cpp
   // Use connection pooling for multiple devices
   QMap<QString, Modbus::TcpPort*> connections;
   
   Modbus::TcpPort* getConnection(const QString& host, int port) {
       QString key = QString("%1:%2").arg(host).arg(port);
       if (!connections.contains(key)) {
           auto* conn = new Modbus::TcpPort();
           conn->setHostName(host);
           conn->setPort(port);
           connections[key] = conn;
       }
       return connections[key];
   }
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
   
   while (retries < maxRetries) {
       Modbus::Response resp = port.readHoldingRegisters(1, 0, 10);
       if (resp.isValid()) break;
       
       QThread::msleep(delay);
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
   port.setStopBits(1);
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
   QMutex busMutex;
   
   void sendRequest() {
       QMutexLocker locker(&busMutex);
       // Only one master can transmit at a time
       Modbus::Response resp = port.readCoils(1, 0, 100);
   }
   ```

5. **Cable and Termination**
   - Use twisted pair cable for RS-485
   - Add 120Ω termination resistors at both ends of RS-485 bus
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
   bool validateFrame(const QByteArray& frame) {
       if (frame[0] != ':') return false;
       if (frame.size() < 11) return false;  // Minimum frame size
       
       // Extract LRC and data
       QByteArray data = frame.mid(1, frame.size() - 5);
       QByteArray lrcStr = frame.mid(frame.size() - 4, 2);
       
       // Calculate and compare
       uint8_t expectedLRC = calculateLRC(data);
       uint8_t receivedLRC = lrcStr.toUShort(nullptr, 16);
       
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
       qint64 lastPoll;
   };
   
   void pollDevices(QVector<PollItem>& items) {
       qint64 now = QDateTime::currentMSecsSinceEpoch();
       
       // Sort by priority and interval
       std::sort(items.begin(), items.end(), [now](const PollItem& a, const PollItem& b) {
           qint64 aDue = a.lastPoll + a.interval - now;
           qint64 bDue = b.lastPoll + b.interval - now;
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
   // Enable detailed logging for troubleshooting
   port.setDebugEnabled(true);
   
   // Custom logging
   connect(&port, &Modbus::Port::frameTransmitted, 
           [](const QByteArray& frame) {
       qDebug() << "TX:" << frame.toHex(' ');
   });
   
   connect(&port, &Modbus::Port::frameReceived,
           [](const QByteArray& frame) {
       qDebug() << "RX:" << frame.toHex(' ');
   });
   ```

5. **Thread Safety**
   ```cpp
   // Use separate port instances per thread
   // or protect with mutex
   class ThreadSafeModbusPort {
       Modbus::TcpPort port;
       QMutex mutex;
       
   public:
       Modbus::Response readRegisters(int addr, int count) {
           QMutexLocker locker(&mutex);
           return port.readHoldingRegisters(1, addr, count);
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

This document provides comprehensive protocol-level details for implementing Modbus TCP, RTU, and ASCII communications using the ModbusLib C++ library. Key takeaways:

- **TCP** is best for modern Ethernet networks with minimal overhead
- **RTU** provides efficient binary communication over serial links
- **ASCII** offers human-readable debugging for serial applications

Each protocol has specific timing, framing, and error checking requirements that must be followed for reliable communication. The ModbusLib library abstracts these details while providing full control when needed.

For implementation details and API reference, see:
- \ref client_guide "Client Implementation Guide"
- \ref server_guide "Server Implementation Guide"  
- \ref api_reference "API Reference"
