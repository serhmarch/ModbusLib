#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <vector>

#include <Modbus.h>
#include <ModbusTcpPort.h>
#include <ModbusSerialPort.h>

using namespace testing;
using namespace Modbus;

TEST(ModbusTest, crc16)
{
    EXPECT_EQ(crc16(reinterpret_cast<const uint8_t*>("\xDE\xAD\xBE\xAF"), 4), 0x319A);
    EXPECT_EQ(crc16(reinterpret_cast<const uint8_t*>("\x01\x03\x00\x00\x00\x0A"), 6), 0xCDC5);
}

TEST(ModbusTest, readMemBits)
{
    const uint16_t mem[] = { 0x01FC, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
    StatusCode status;
    uint16_t v;

    v = 0;
    status = readMemBits(0, 8, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(v, 0x00FC);

    v = 0;
    status = readMemBits(0, 10, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(v, 0x01FC);

    v = 0;
    status = readMemBits(1, 8, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(v, 0x00FE);

    v = 0;
    status = readMemBits(1, 10, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(v, 0x00FE);

    status = readMemBits(sizeof(mem)*MB_BYTE_SZ_BITES, 1, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_BadIllegalDataAddress);

    status = readMemBits(sizeof(mem)*MB_BYTE_SZ_BITES-2, 3, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_BadIllegalDataAddress);

    status = readMemBits(sizeof(mem)*MB_BYTE_SZ_BITES-2, 2, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_Good);
}

TEST(ModbusTest, writeMemBits)
{
    uint16_t mem[16];
    StatusCode status;
    uint16_t v;

    memset(&mem, 0, sizeof(mem));
    v = 0x00FF;
    status = writeMemBits(0, 8, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(mem[0], 0x00FF);

    memset(&mem, 0, sizeof(mem));
    v = 0x0FFF;
    status = writeMemBits(0, 10, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(mem[0], 0x03FF);

    memset(&mem, 0, sizeof(mem));
    v = 0x00FF;
    status = writeMemBits(1, 8, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(mem[0], 0x01FE);

    memset(&mem, 0, sizeof(mem));
    v = 0x0FFF;
    status = writeMemBits(1, 10, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    ASSERT_EQ(status, Status_Good);
    EXPECT_EQ(mem[0], 0x07FE);

    status = writeMemBits(sizeof(mem)*MB_BYTE_SZ_BITES, 1, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_BadIllegalDataAddress);

    status = writeMemBits(sizeof(mem)*MB_BYTE_SZ_BITES-2, 3, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_BadIllegalDataAddress);

    status = writeMemBits(sizeof(mem)*MB_BYTE_SZ_BITES-2, 2, &v, mem, sizeof(mem)*MB_BYTE_SZ_BITES);
    EXPECT_EQ(status, Status_Good);
}

TEST(ModbusTest, bytesToAscii)
{
    const uint8_t bytes[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    std::vector<uint8_t> ascii(sizeof(bytes)*2);
    uint32_t c = bytesToAscii(bytes, ascii.data(), sizeof(bytes));
    ASSERT_EQ(c, ascii.size());
    EXPECT_THAT(ascii, ElementsAre( '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'));
}


TEST(ModbusTest, asciiToBytes)
{
    const uint8_t ascii[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::vector<uint8_t> bytes(sizeof(ascii)/2);
    uint32_t c = asciiToBytes(ascii, bytes.data(), sizeof(ascii));
    ASSERT_EQ(c, bytes.size());
    EXPECT_THAT(bytes, ElementsAre(0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF));
}


TEST(ModbusTest, toModbusString)
{
    EXPECT_EQ(toModbusString(0), "0");
    EXPECT_EQ(toModbusString(1), "1");
    EXPECT_EQ(toModbusString(-1), "-1");
}

TEST(ModbusTest, bytesToString)
{
    EXPECT_EQ(bytesToString(reinterpret_cast<const uint8_t*>("\x01\x03\x00\x00\x00\x0A"), 6), StringLiteral("01 03 00 00 00 0A "));
}

TEST(ModbusTest, asciiToString)
{
    EXPECT_EQ(asciiToString(reinterpret_cast<const uint8_t*>(":01030000000A\r\n"), 15), StringLiteral(": 01 03 00 00 00 0A CR LF "));
}

TEST(ModbusTest, createPortTcp)
{
    TcpSettings tcp;
    tcp.host    = "localhost";
    tcp.port    = STANDARD_TCP_PORT;
    tcp.timeout = 5000;
    ModbusPort *port = createPort(TCP, &tcp, false);
    ASSERT_NE(port, nullptr);
    EXPECT_EQ(port->type(), TCP);
    EXPECT_STREQ(reinterpret_cast<ModbusTcpPort*>(port)->host(), tcp.host);
    EXPECT_EQ(reinterpret_cast<ModbusTcpPort*>(port)->port(), tcp.port);
    EXPECT_EQ(reinterpret_cast<ModbusTcpPort*>(port)->timeout(), tcp.timeout);
    delete port;
}


TEST(ModbusTest, createPortRtu)
{
    SerialSettings ser;
    ser.portName         = StringLiteral("COM1");
    ser.baudRate         = 19200;
    ser.dataBits         = 7;
    ser.parity           = Modbus::OddParity;
    ser.stopBits         = Modbus::OneAndHalfStop;
    ser.flowControl      = Modbus::NoFlowControl;
    ser.timeoutFirstByte = 5000;
    ser.timeoutInterByte = 100;

    ModbusPort *port = createPort(RTU, &ser, false);
    ASSERT_NE(port, nullptr);
    EXPECT_EQ(port->type(), RTU);
    EXPECT_STREQ(reinterpret_cast<ModbusSerialPort*>(port)->portName(), ser.portName);
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->baudRate        (), ser.baudRate        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->dataBits        (), ser.dataBits        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->parity          (), ser.parity          );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->stopBits        (), ser.stopBits        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->flowControl     (), ser.flowControl     );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->timeoutFirstByte(), ser.timeoutFirstByte);
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->timeoutInterByte(), ser.timeoutInterByte);
    delete port;
}

TEST(ModbusTest, createPortAsc)
{
    SerialSettings ser;
    ser.portName         = StringLiteral("COM1");
    ser.baudRate         = 115200;
    ser.dataBits         = 7;
    ser.parity           = Modbus::EvenParity;
    ser.stopBits         = Modbus::TwoStop;
    ser.flowControl      = Modbus::HardwareControl;
    ser.timeoutFirstByte = 5000;
    ser.timeoutInterByte = 100;


    ModbusPort *port = createPort(ASC, &ser, false);
    ASSERT_NE(port, nullptr);
    EXPECT_EQ(port->type(), ASC);
    EXPECT_STREQ(reinterpret_cast<ModbusSerialPort*>(port)->portName(), ser.portName);
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->baudRate        (), ser.baudRate        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->dataBits        (), ser.dataBits        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->parity          (), ser.parity          );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->stopBits        (), ser.stopBits        );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->flowControl     (), ser.flowControl     );
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->timeoutFirstByte(), ser.timeoutFirstByte);
    EXPECT_EQ(reinterpret_cast<ModbusSerialPort*>(port)->timeoutInterByte(), ser.timeoutInterByte);
    delete port;
}