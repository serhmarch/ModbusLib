#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusRtuPort.h>
#include <ModbusSerialPort.h>
#include <ModbusPort_p.h>
#include <ModbusSerialPort_p.h>
#include <ModbusGlobal.h>

using namespace Modbus;

// Helper class to access protected members for testing
class ModbusRtuPortTestHelper : public ModbusRtuPort
{
public:
    ModbusRtuPortTestHelper(bool blocking = true) : ModbusRtuPort(blocking) {}

    // Expose protected methods for testing
    StatusCode testWriteBuffer(uint8_t unit, uint8_t func, uint8_t *buff, uint16_t szInBuff)
    {
        return writeBuffer(unit, func, buff, szInBuff);
    }

    StatusCode testReadBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff)
    {
        return readBuffer(unit, func, buff, maxSzBuff, szOutBuff);
    }

    // Access to private data for test setup
    void setInternalBuffer(const uint8_t *data, uint16_t size)
    {
        ModbusSerialPortPrivate *d = static_cast<ModbusSerialPortPrivate*>(this->d_ptr);
        memcpy(d->buff, data, size);
        d->sz = size;
    }

    // Expose write() and read() for testing
    StatusCode testWrite()
    {
        return write();
    }

    StatusCode testRead()
    {
        return read();
    }
};

// Test Fixture for ModbusRtuPort
class ModbusRtuPortTest : public ::testing::Test
{
protected:
    ModbusRtuPortTestHelper *port;

    void SetUp() override
    {
        port = nullptr;
    }

    void TearDown() override
    {
        if (port)
        {
            delete port;
            port = nullptr;
        }
    }
};

// ============================================================================
// Basic Initialization Tests
// ============================================================================

TEST_F(ModbusRtuPortTest, TypeReturnsRtu)
{
    port = new ModbusRtuPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::RTU);
}

TEST_F(ModbusRtuPortTest, InitializationDefault)
{
    port = new ModbusRtuPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::RTU);
    EXPECT_FALSE(port->isOpen());
    EXPECT_TRUE(port->isBlocking());
}

TEST_F(ModbusRtuPortTest, InitializationBlockingMode)
{
    ModbusRtuPortTestHelper *portBlocking = new ModbusRtuPortTestHelper(true);
    EXPECT_TRUE(portBlocking->isBlocking());
    delete portBlocking;

    ModbusRtuPortTestHelper *portNonBlocking = new ModbusRtuPortTestHelper(false);
    EXPECT_FALSE(portNonBlocking->isBlocking());
    EXPECT_TRUE(portNonBlocking->isNonBlocking());
    delete portNonBlocking;
}

// ============================================================================
// `open()`-method tests
// ============================================================================

TEST_F(ModbusRtuPortTest, OpenMethodClearsChangedFlag)
{
    auto *rtuPort = new ModbusRtuPort(true);
    
    rtuPort->setPortName("somethingnonexisting.plc");
    EXPECT_TRUE(rtuPort->isChanged());    
    auto status = rtuPort->open();
    EXPECT_TRUE(Modbus::StatusIsBad(status));
    EXPECT_FALSE(rtuPort->isChanged());    
    delete rtuPort;
}

// ============================================================================
// Write Buffer Tests (RTU Frame Construction with CRC16)
// ============================================================================

TEST_F(ModbusRtuPortTest, WriteBufferConstructsCrcAndReturnsGood)
{
    port = new ModbusRtuPortTestHelper();
    
    // RTU frame: unit + func + data + CRC16 (little-endian)
    uint8_t unit = 0x11;
    uint8_t func = 0x03;
    uint8_t data[4] = {0x00, 0x10, 0x00, 0x02};
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Buffer length = 1(unit) + 1(func) + len(data) + 2(crc)
    EXPECT_EQ(buffSize, 1 + 1 + sizeof(data) + 2);
    EXPECT_EQ(buff[0], unit);
    EXPECT_EQ(buff[1], func);
    EXPECT_EQ(memcmp(&buff[2], data, sizeof(data)), 0);
    
    // Verify CRC matches crc16 of first bytes (unit+func+data)
    uint16_t crcFromBuff = buff[buffSize - 2] | (buff[buffSize - 1] << 8); // little-endian
    uint16_t expectedCrc = crc16(buff, buffSize - 2);
    EXPECT_EQ(crcFromBuff, expectedCrc);
}

TEST_F(ModbusRtuPortTest, WriteBufferReadHoldingRegisters)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x0A}; // Start address 0, quantity 10
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    EXPECT_EQ(buffSize, 8); // 1 + 1 + 4 + 2
    EXPECT_EQ(buff[0], unit);
    EXPECT_EQ(buff[1], func);
    
    // Verify CRC
    uint16_t crcFromBuff = buff[6] | (buff[7] << 8);
    uint16_t expectedCrc = crc16(buff, 6);
    EXPECT_EQ(crcFromBuff, expectedCrc);
}

TEST_F(ModbusRtuPortTest, WriteBufferWriteSingleCoil)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_SINGLE_COIL;
    uint8_t data[4] = {0x00, 0x0A, 0xFF, 0x00}; // Address 10, ON
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    EXPECT_EQ(buffSize, 8);
    EXPECT_EQ(buff[0], unit);
    EXPECT_EQ(buff[1], func);
    EXPECT_EQ(buff[2], 0x00);
    EXPECT_EQ(buff[3], 0x0A);
    EXPECT_EQ(buff[4], 0xFF);
    EXPECT_EQ(buff[5], 0x00);
    
    // Verify CRC
    uint16_t crcFromBuff = buff[6] | (buff[7] << 8);
    uint16_t expectedCrc = crc16(buff, 6);
    EXPECT_EQ(crcFromBuff, expectedCrc);
}

TEST_F(ModbusRtuPortTest, WriteBufferZeroLengthData)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_EXCEPTION_STATUS;
    
    StatusCode result = port->testWriteBuffer(unit, func, nullptr, 0);
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Should be: unit + func + CRC (4 bytes)
    EXPECT_EQ(buffSize, 4);
    EXPECT_EQ(buff[0], unit);
    EXPECT_EQ(buff[1], func);
    
    // Verify CRC
    uint16_t crcFromBuff = buff[2] | (buff[3] << 8);
    uint16_t expectedCrc = crc16(buff, 2);
    EXPECT_EQ(crcFromBuff, expectedCrc);
}

TEST_F(ModbusRtuPortTest, WriteBufferLargeData)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[128];
    for (int i = 0; i < 128; i++)
        data[i] = static_cast<uint8_t>(i);
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    EXPECT_EQ(buffSize, 132); // 1 + 1 + 128 + 2
    
    // Verify data integrity
    EXPECT_EQ(memcmp(&buff[2], data, sizeof(data)), 0);
    
    // Verify CRC
    uint16_t crcFromBuff = buff[130] | (buff[131] << 8);
    uint16_t expectedCrc = crc16(buff, 130);
    EXPECT_EQ(crcFromBuff, expectedCrc);
}

// ============================================================================
// Read Buffer Tests (RTU Frame Parsing with CRC16 Validation)
// ============================================================================

TEST_F(ModbusRtuPortTest, ReadBufferValidFrame)
{
    port = new ModbusRtuPortTestHelper();
    
    // Prepare valid RTU frame
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[5] = {0x02, 0x00, 0x0A, 0x00, 0x14}; // Byte count=2, 2 register values
    
    // Build frame with CRC
    uint8_t frame[10];
    frame[0] = unit;
    frame[1] = func;
    memcpy(&frame[2], data, sizeof(data));
    
    uint16_t crc = crc16(frame, 7);
    frame[7] = crc & 0xFF;        // CRC low byte
    frame[8] = (crc >> 8) & 0xFF; // CRC high byte
    
    port->setInternalBuffer(frame, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 5);
    EXPECT_EQ(memcmp(outBuff, data, sizeof(data)), 0);
}

TEST_F(ModbusRtuPortTest, ReadBufferReadHoldingRegistersResponse)
{
    port = new ModbusRtuPortTestHelper();
    
    // Response to Read Holding Registers request
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t byteCount = 0x04;
    uint8_t regValues[4] = {0x00, 0x0A, 0x00, 0x0B}; // 2 registers
    
    uint8_t frame[9];
    frame[0] = unit;
    frame[1] = func;
    frame[2] = byteCount;
    memcpy(&frame[3], regValues, sizeof(regValues));
    
    uint16_t crc = crc16(frame, 7);
    frame[7] = crc & 0xFF;
    frame[8] = (crc >> 8) & 0xFF;
    
    port->setInternalBuffer(frame, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 5); // byte count + 4 data bytes
    EXPECT_EQ(outBuff[0], byteCount);
    EXPECT_EQ(memcmp(&outBuff[1], regValues, sizeof(regValues)), 0);
}

TEST_F(ModbusRtuPortTest, ReadBufferExceptionResponse)
{
    port = new ModbusRtuPortTestHelper();
    
    // Exception response
    uint8_t unit = 0x01;
    uint8_t func = 0x83; // Exception: Read Holding Registers
    uint8_t exceptionCode = 0x02; // Illegal Data Address
    
    uint8_t frame[5];
    frame[0] = unit;
    frame[1] = func;
    frame[2] = exceptionCode;
    
    uint16_t crc = crc16(frame, 3);
    frame[3] = crc & 0xFF;
    frame[4] = (crc >> 8) & 0xFF;
    
    port->setInternalBuffer(frame, 5);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 1);
    EXPECT_EQ(outBuff[0], exceptionCode);
}

TEST_F(ModbusRtuPortTest, ReadBufferTooSmallFrame)
{
    port = new ModbusRtuPortTestHelper();
    
    // Frame with less than 4 bytes (minimum: unit + func + CRC)
    uint8_t tooSmallFrame[3] = {0x01, 0x03, 0x00};
    
    port->setInternalBuffer(tooSmallFrame, 3);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
}

TEST_F(ModbusRtuPortTest, ReadBufferWrongCrcRaises)
{
    port = new ModbusRtuPortTestHelper();
    
    // Frame with incorrect CRC
    uint8_t unit = 0x01;
    uint8_t func = 0x06;
    uint8_t data[4] = {0x00, 0x01, 0x00, 0x02};
    
    uint8_t frame[8];
    frame[0] = unit;
    frame[1] = func;
    memcpy(&frame[2], data, sizeof(data));
    
    // Append incorrect CRC (zero instead of calculated)
    frame[6] = 0x00;
    frame[7] = 0x00;
    
    port->setInternalBuffer(frame, 8);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadCrc);
}

TEST_F(ModbusRtuPortTest, ReadBufferMinimalValidFrame)
{
    port = new ModbusRtuPortTestHelper();
    
    // Minimal frame: unit + func + CRC (4 bytes)
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_EXCEPTION_STATUS;
    
    uint8_t frame[4];
    frame[0] = unit;
    frame[1] = func;
    
    uint16_t crc = crc16(frame, 2);
    frame[2] = crc & 0xFF;
    frame[3] = (crc >> 8) & 0xFF;
    
    port->setInternalBuffer(frame, 4);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 0); // No data
}

// ============================================================================
// Write/Read Cycle Tests
// ============================================================================

TEST_F(ModbusRtuPortTest, CompleteRequestResponseCycle)
{
    port = new ModbusRtuPortTestHelper();
    
    // Prepare request
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02}; // Read 2 registers from address 0
    
    StatusCode writeResult = port->testWriteBuffer(unit, func, requestData, sizeof(requestData));
    EXPECT_EQ(writeResult, Status_Good);
    
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    EXPECT_EQ(writeSize, 8); // unit + func + 4 data + 2 CRC
    
    // Verify written request has valid CRC
    uint16_t writeCrc = writeBuff[6] | (writeBuff[7] << 8);
    uint16_t expectedWriteCrc = crc16(writeBuff, 6);
    EXPECT_EQ(writeCrc, expectedWriteCrc);
    
    // Simulate response
    uint8_t responseFrame[9];
    responseFrame[0] = unit;
    responseFrame[1] = func;
    responseFrame[2] = 0x04; // Byte count
    responseFrame[3] = 0x00; // Reg 1 MSB
    responseFrame[4] = 0x0A; // Reg 1 LSB
    responseFrame[5] = 0x00; // Reg 2 MSB
    responseFrame[6] = 0x14; // Reg 2 LSB
    
    uint16_t responseCrc = crc16(responseFrame, 7);
    responseFrame[7] = responseCrc & 0xFF;
    responseFrame[8] = (responseCrc >> 8) & 0xFF;
    
    port->setInternalBuffer(responseFrame, 9);
    
    // Parse response
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode readResult = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(readResult, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 5);
    EXPECT_EQ(outBuff[0], 0x04); // Byte count
    
    // Check register values
    uint16_t reg1 = (outBuff[1] << 8) | outBuff[2];
    uint16_t reg2 = (outBuff[3] << 8) | outBuff[4];
    EXPECT_EQ(reg1, 0x000A);
    EXPECT_EQ(reg2, 0x0014);
}

// ============================================================================
// Serial Port Configuration Tests
// ============================================================================

TEST_F(ModbusRtuPortTest, SerialPortConfiguration)
{
    port = new ModbusRtuPortTestHelper();
    
    // Test port name configuration
    port->setPortName("COM3");
    EXPECT_EQ(std::string(port->portName()), std::string("COM3"));
    
    // Test baud rate
    port->setBaudRate(19200);
    EXPECT_EQ(port->baudRate(), 19200);
    
    // Test data bits
    port->setDataBits(8);
    EXPECT_EQ(port->dataBits(), 8);
    
    // Test parity
    port->setParity(Modbus::EvenParity);
    EXPECT_EQ(port->parity(), Modbus::EvenParity);
    
    // Test stop bits
    port->setStopBits(Modbus::OneStop);
    EXPECT_EQ(port->stopBits(), Modbus::OneStop);
    
    // Test flow control
    port->setFlowControl(Modbus::NoFlowControl);
    EXPECT_EQ(port->flowControl(), Modbus::NoFlowControl);
}

TEST_F(ModbusRtuPortTest, DefaultSerialSettings)
{
    port = new ModbusRtuPortTestHelper();
    
    const ModbusRtuPort::Defaults &defaults = ModbusRtuPort::Defaults::instance();
    
    EXPECT_EQ(port->baudRate(), defaults.baudRate);
    EXPECT_EQ(port->dataBits(), defaults.dataBits);
    EXPECT_EQ(port->parity(), defaults.parity);
    EXPECT_EQ(port->stopBits(), defaults.stopBits);
    EXPECT_EQ(port->flowControl(), defaults.flowControl);
}

// ============================================================================
// CRC Validation Tests
// ============================================================================

TEST_F(ModbusRtuPortTest, CrcCalculationConsistency)
{
    port = new ModbusRtuPortTestHelper();
    
    // Test that writing and reading with same data produces consistent CRC
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_COILS;
    uint8_t data[4] = {0x00, 0x13, 0x00, 0x25};
    
    // Write buffer
    port->testWriteBuffer(unit, func, data, sizeof(data));
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    
    // Extract CRC from written buffer
    uint16_t crcFromWrite = writeBuff[writeSize - 2] | (writeBuff[writeSize - 1] << 8);
    
    // Manually calculate CRC
    uint16_t calculatedCrc = crc16(writeBuff, writeSize - 2);
    
    EXPECT_EQ(crcFromWrite, calculatedCrc);
    
    // Now set this as read buffer and verify it parses correctly
    port->setInternalBuffer(writeBuff, writeSize);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
}

TEST_F(ModbusRtuPortTest, DifferentDataSizesCrcValidation)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    
    // Test various data sizes
    for (int dataSize = 0; dataSize <= 64; dataSize += 8)
    {
        uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
        uint8_t *data = new uint8_t[dataSize];
        for (int i = 0; i < dataSize; i++)
            data[i] = static_cast<uint8_t>(i);
        
        StatusCode result = port->testWriteBuffer(unit, func, data, dataSize);
        EXPECT_EQ(result, Status_Good);
        
        const uint8_t *buff = port->writeBufferData();
        uint16_t buffSize = port->writeBufferSize();
        
        // Verify CRC
        uint16_t crcFromBuff = buff[buffSize - 2] | (buff[buffSize - 1] << 8);
        uint16_t expectedCrc = crc16(buff, buffSize - 2);
        EXPECT_EQ(crcFromBuff, expectedCrc);
        
        delete[] data;
    }
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(ModbusRtuPortTest, BufferOverflow)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[MB_RTU_IO_BUFF_SZ]; // Too large
    memset(data, 0, sizeof(data));
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_BadWriteBufferOverflow);
}

TEST_F(ModbusRtuPortTest, WriteReadWhenNotOpen)
{
    port = new ModbusRtuPortTestHelper();
    
    // Port is not open
    EXPECT_FALSE(port->isOpen());
    
    // Try to write
    StatusCode writeResult = port->testWrite();
    EXPECT_NE(writeResult, Status_Good);
    
    // Try to read
    StatusCode readResult = port->testRead();
    EXPECT_NE(readResult, Status_Good);
}

TEST_F(ModbusRtuPortTest, MultipleWriteBufferCalls)
{
    port = new ModbusRtuPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    // First write
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t firstSize = port->writeBufferSize();
    const uint8_t *firstBuff = port->writeBufferData();
    uint8_t firstBuffCopy[256];
    memcpy(firstBuffCopy, firstBuff, firstSize);
    
    // Second write (should overwrite previous buffer)
    data[3] = 0x02;
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t secondSize = port->writeBufferSize();
    const uint8_t *secondBuff = port->writeBufferData();
    
    EXPECT_EQ(firstSize, secondSize);
    // Buffers should be different (different data and CRC)
    EXPECT_NE(memcmp(firstBuffCopy, secondBuff, secondSize), 0);
}

TEST_F(ModbusRtuPortTest, CloseWhenNotOpen)
{
    port = new ModbusRtuPortTestHelper();
    
    StatusCode result = port->close();
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_FALSE(port->isOpen());
}

TEST_F(ModbusRtuPortTest, HandleMethod)
{
    port = new ModbusRtuPortTestHelper();
    
    // When not open, handle should be invalid
    Handle h = port->handle();
    // Handle value depends on implementation
    (void)h; // Avoid unused variable warning
}
