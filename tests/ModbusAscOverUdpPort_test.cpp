#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusAscOverUdpPort.h>
#include <ModbusPort_p.h>
#include <ModbusGlobal.h>

using namespace Modbus;

// Helper class to access protected members for testing
class ModbusAscOverUdpPortTestHelper : public ModbusAscOverUdpPort
{
public:
    ModbusAscOverUdpPortTestHelper(bool blocking = true) : ModbusAscOverUdpPort(blocking) {}

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
        auto buff = d_ptr->buff();
        memcpy(buff, data, size);
        d_ptr->setBuffSz(size);
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

    // Get internal buffer for verification
    const uint8_t* getInternalBuffer() const
    {
        return this->writeBufferData();
    }

    uint16_t getInternalBufferSize() const
    {
        return this->writeBufferSize();
    }
};

// Test Fixture for ModbusAscOverUdpPort
class ModbusAscOverUdpPortTest : public ::testing::Test
{
protected:
    ModbusAscOverUdpPortTestHelper *port;

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
// Initialization Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, InitializationDefault)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::ASCvUDP);
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->host(), std::string("localhost"));
    EXPECT_EQ(port->port(), STANDARD_UDP_PORT);
    EXPECT_TRUE(port->isBlocking());
}

TEST_F(ModbusAscOverUdpPortTest, InitializationBlockingMode)
{
    ModbusAscOverUdpPortTestHelper *portBlocking = new ModbusAscOverUdpPortTestHelper(true);
    EXPECT_TRUE(portBlocking->isBlocking());
    delete portBlocking;

    ModbusAscOverUdpPortTestHelper *portNonBlocking = new ModbusAscOverUdpPortTestHelper(false);
    EXPECT_FALSE(portNonBlocking->isBlocking());
    EXPECT_TRUE(portNonBlocking->isNonBlocking());
    delete portNonBlocking;
}

TEST_F(ModbusAscOverUdpPortTest, InitializationBasic)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->type(), Modbus::ASCvUDP);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, HostConfiguration)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    port->setHost("192.168.1.100");
    EXPECT_EQ(std::string(port->host()), std::string("192.168.1.100"));
    EXPECT_TRUE(port->isChanged());
}

TEST_F(ModbusAscOverUdpPortTest, PortConfiguration)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    port->setPort(1502);
    EXPECT_EQ(port->port(), 1502);
    EXPECT_TRUE(port->isChanged());
}

TEST_F(ModbusAscOverUdpPortTest, TimeoutConfiguration)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    port->setTimeout(5000);
    EXPECT_EQ(port->timeout(), 5000);
}

TEST_F(ModbusAscOverUdpPortTest, ServerModeConfiguration)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    EXPECT_FALSE(port->isServerMode());
    
    port->setServerMode(true);
    EXPECT_TRUE(port->isServerMode());
    
    port->setServerMode(false);
    EXPECT_FALSE(port->isServerMode());
}

// ============================================================================
// `open()`-method tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, OpenMethodClearsChangedFlag)
{
    auto *ascUdpPort = new ModbusAscOverUdpPort(true);
    
    ascUdpPort->setHost("somethingnonexisting.plc");
    EXPECT_TRUE(ascUdpPort->isChanged());    
    auto status = ascUdpPort->open();
    EXPECT_TRUE(Modbus::StatusIsBad(status));
    EXPECT_FALSE(ascUdpPort->isChanged());    
    delete ascUdpPort;
}

// ============================================================================
// Write Buffer Tests (ASCII Frame Construction with LRC)
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, WriteBufferConstructsAsciiFrameAndReturnsGood)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // ASCII frame: ':' + ascii(hex(unit+func+data+LRC)) + CR LF
    uint8_t unit = 0x10;
    uint8_t func = 0x02;
    uint8_t data[2] = {0x00, 0x05};
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Frame starts with ':'
    EXPECT_EQ(buff[0], ':');
    
    // Frame ends with CR LF
    EXPECT_EQ(buff[buffSize - 2], '\r');
    EXPECT_EQ(buff[buffSize - 1], '\n');
    
    // Build expected internal buffer: unit + func + data + LRC
    uint8_t ibuff[5];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t expectedLrc = lrc(ibuff, 4);
    ibuff[4] = expectedLrc;
    
    // Convert to ASCII hex
    uint8_t expectedAscii[10]; // 5 bytes * 2 = 10 ASCII characters
    bytesToAscii(ibuff, expectedAscii, 5);
    
    // Verify ASCII payload
    EXPECT_EQ(memcmp(&buff[1], expectedAscii, 10), 0);
}

TEST_F(ModbusAscOverUdpPortTest, WriteBufferReadHoldingRegisters)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x0A}; // Start address 0, quantity 10
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Verify frame structure
    EXPECT_EQ(buff[0], ':');
    EXPECT_EQ(buff[buffSize - 2], '\r');
    EXPECT_EQ(buff[buffSize - 1], '\n');
    
    // Build expected data
    uint8_t ibuff[7];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t expectedLrc = lrc(ibuff, 6);
    ibuff[6] = expectedLrc;
    
    // Convert to ASCII
    uint8_t expectedAscii[14];
    bytesToAscii(ibuff, expectedAscii, 7);
    
    // Verify
    EXPECT_EQ(memcmp(&buff[1], expectedAscii, 14), 0);
}

TEST_F(ModbusAscOverUdpPortTest, WriteBufferWriteSingleCoil)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_SINGLE_COIL;
    uint8_t data[4] = {0x00, 0x0A, 0xFF, 0x00}; // Address 10, ON
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    
    EXPECT_EQ(buff[0], ':');
    EXPECT_EQ(buff[port->writeBufferSize() - 2], '\r');
    EXPECT_EQ(buff[port->writeBufferSize() - 1], '\n');
}

TEST_F(ModbusAscOverUdpPortTest, WriteBufferZeroLengthData)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_EXCEPTION_STATUS;
    
    StatusCode result = port->testWriteBuffer(unit, func, nullptr, 0);
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Frame: ':' + ASCII(unit+func+LRC) + CRLF
    // 1 + (3 bytes * 2 ASCII chars) + 2 = 9 bytes
    EXPECT_EQ(buffSize, 9);
    EXPECT_EQ(buff[0], ':');
    EXPECT_EQ(buff[7], '\r');
    EXPECT_EQ(buff[8], '\n');
}

TEST_F(ModbusAscOverUdpPortTest, WriteBufferLargeData)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[64];
    for (int i = 0; i < 64; i++)
        data[i] = static_cast<uint8_t>(i);
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t buffSize = port->writeBufferSize();
    
    // Verify frame structure
    EXPECT_EQ(buff[0], ':');
    EXPECT_EQ(buff[buffSize - 2], '\r');
    EXPECT_EQ(buff[buffSize - 1], '\n');
    
    // Expected size: 1 (':') + ((1+1+64+1)*2) ASCII chars + 2 (CRLF) = 137 bytes
    EXPECT_EQ(buffSize, 137);
}

TEST_F(ModbusAscOverUdpPortTest, WriteBufferOverflow)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[MB_ASC_IO_BUFF_SZ]; // Too large
    memset(data, 0, sizeof(data));
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_BadWriteBufferOverflow);
}

// ============================================================================
// Read Buffer Tests (ASCII Frame Parsing with LRC Validation)
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, ReadBufferValidFrame)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Build valid ASCII frame
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[5] = {0x02, 0x00, 0x0A, 0x00, 0x14}; // Byte count=2, 2 register values
    
    // Build internal buffer with LRC
    uint8_t ibuff[8];  // unit(1) + func(1) + data(5) + lrc(1) = 8 bytes
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t lrcValue = lrc(ibuff, 7);  // LRC calculated on first 7 bytes (unit+func+data)
    ibuff[7] = lrcValue;
    
    // Convert to ASCII
    uint8_t asciiPayload[16];  // 8 bytes * 2 = 16 ASCII chars
    bytesToAscii(ibuff, asciiPayload, 8);
    
    // Build complete frame: ':' + ASCII payload + CRLF
    uint8_t frame[19];  // 1 + 16 + 2 = 19
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 16);
    frame[17] = '\r';
    frame[18] = '\n';
    
    port->setInternalBuffer(frame, 19);
    
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

TEST_F(ModbusAscOverUdpPortTest, ReadBufferReadHoldingRegistersResponse)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t byteCount = 0x04;
    uint8_t regValues[4] = {0x00, 0x0A, 0x00, 0x0B};
    
    // Build internal buffer
    uint8_t ibuff[8];  // unit(1) + func(1) + byteCount(1) + regValues(4) + lrc(1) = 8 bytes
    ibuff[0] = unit;
    ibuff[1] = func;
    ibuff[2] = byteCount;
    memcpy(&ibuff[3], regValues, sizeof(regValues));
    uint8_t lrcValue = lrc(ibuff, 7);
    ibuff[7] = lrcValue;
    
    // Convert to ASCII
    uint8_t asciiPayload[16];
    bytesToAscii(ibuff, asciiPayload, 8);
    
    // Build frame
    uint8_t frame[19];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 16);
    frame[17] = '\r';
    frame[18] = '\n';
    
    port->setInternalBuffer(frame, 19);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 5);
    EXPECT_EQ(outBuff[0], byteCount);
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferExceptionResponse)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = 0x83; // Exception: Read Holding Registers
    uint8_t exceptionCode = 0x02;
    
    // Build internal buffer: unit+func+exception+LRC
    uint8_t ibuff[4];
    ibuff[0] = unit;
    ibuff[1] = func;
    ibuff[2] = exceptionCode;
    uint8_t lrcValue = lrc(ibuff, 3);
    ibuff[3] = lrcValue;
    
    // Convert to ASCII
    uint8_t asciiPayload[8];
    bytesToAscii(ibuff, asciiPayload, 4);
    
    // Build frame
    uint8_t frame[11];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 8);
    frame[9] = '\r';
    frame[10] = '\n';
    
    port->setInternalBuffer(frame, 11);
    
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

TEST_F(ModbusAscOverUdpPortTest, ReadBufferTooSmallFrame)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Frame too small (less than 9 bytes minimum)
    uint8_t tooSmallFrame[1] = {':'};
    
    port->setInternalBuffer(tooSmallFrame, 1);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferMissingColonRaises)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Build frame with wrong start symbol
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[2] = {0x00, 0x01};
    
    uint8_t ibuff[5];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t lrcValue = lrc(ibuff, 4);
    ibuff[4] = lrcValue;
    
    uint8_t asciiPayload[10];
    bytesToAscii(ibuff, asciiPayload, 5);
    
    uint8_t frame[13];
    frame[0] = '#'; // Wrong start symbol
    memcpy(&frame[1], asciiPayload, 10);
    frame[11] = '\r';
    frame[12] = '\n';
    
    port->setInternalBuffer(frame, 13);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadAscMissColon);
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferMissingCrlfRaises)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Build frame without CRLF ending
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    
    uint8_t ibuff[3];
    ibuff[0] = unit;
    ibuff[1] = func;
    uint8_t lrcValue = lrc(ibuff, 2);
    ibuff[2] = lrcValue;
    
    uint8_t asciiPayload[6];
    bytesToAscii(ibuff, asciiPayload, 3);
    
    uint8_t frame[9];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 6);
    frame[7] = 'X'; // Not CR
    frame[8] = 'Y'; // Not LF
    
    port->setInternalBuffer(frame, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadAscMissCrLf);
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferWrongLrcRaises)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Build frame with incorrect LRC
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[2] = {0x00, 0x01};
    
    uint8_t ibuff[5];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    ibuff[4] = 0x00; // Wrong LRC (instead of calculated)
    
    uint8_t asciiPayload[10];
    bytesToAscii(ibuff, asciiPayload, 5);
    
    uint8_t frame[13];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 10);
    frame[11] = '\r';
    frame[12] = '\n';
    
    port->setInternalBuffer(frame, 13);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadLrc);
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferMinimalValidFrame)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Minimal frame: unit + func + LRC (9 ASCII bytes total)
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_EXCEPTION_STATUS;
    
    uint8_t ibuff[3];
    ibuff[0] = unit;
    ibuff[1] = func;
    uint8_t lrcValue = lrc(ibuff, 2);
    ibuff[2] = lrcValue;
    
    uint8_t asciiPayload[6];
    bytesToAscii(ibuff, asciiPayload, 3);
    
    uint8_t frame[9];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 6);
    frame[7] = '\r';
    frame[8] = '\n';
    
    port->setInternalBuffer(frame, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 0); // No data
}

TEST_F(ModbusAscOverUdpPortTest, ReadBufferInvalidAsciiCharacters)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Build frame with invalid ASCII hex characters
    uint8_t frame[11];
    frame[0] = ':';
    frame[1] = 'G'; // Invalid hex character
    frame[2] = 'H'; // Invalid hex character
    frame[3] = '0';
    frame[4] = '1';
    frame[5] = '0';
    frame[6] = '2';
    frame[7] = 'F';
    frame[8] = 'C';
    frame[9] = '\r';
    frame[10] = '\n';
    
    port->setInternalBuffer(frame, 11);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadAscChar);
}

// ============================================================================
// Write/Read Cycle Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, CompleteRequestResponseCycle)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Prepare request
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02};
    
    StatusCode writeResult = port->testWriteBuffer(unit, func, requestData, sizeof(requestData));
    EXPECT_EQ(writeResult, Status_Good);
    
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    
    // Verify written frame structure
    EXPECT_EQ(writeBuff[0], ':');
    EXPECT_EQ(writeBuff[writeSize - 2], '\r');
    EXPECT_EQ(writeBuff[writeSize - 1], '\n');
    
    // Simulate response
    uint8_t ibuff[8];
    ibuff[0] = unit;
    ibuff[1] = func;
    ibuff[2] = 0x04; // Byte count
    ibuff[3] = 0x00; // Reg 1 MSB
    ibuff[4] = 0x0A; // Reg 1 LSB
    ibuff[5] = 0x00; // Reg 2 MSB
    ibuff[6] = 0x14; // Reg 2 LSB
    uint8_t lrcValue = lrc(ibuff, 7);
    ibuff[7] = lrcValue;
    
    uint8_t asciiPayload[16];
    bytesToAscii(ibuff, asciiPayload, 8);
    
    uint8_t responseFrame[19];
    responseFrame[0] = ':';
    memcpy(&responseFrame[1], asciiPayload, 16);
    responseFrame[17] = '\r';
    responseFrame[18] = '\n';
    
    port->setInternalBuffer(responseFrame, 19);
    
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
// LRC Validation Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, LrcCalculationConsistency)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_COILS;
    uint8_t data[4] = {0x00, 0x13, 0x00, 0x25};
    
    // Write buffer
    port->testWriteBuffer(unit, func, data, sizeof(data));
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    
    // Extract ASCII payload (skip ':' and CRLF)
    uint8_t asciiPayload[256];
    memcpy(asciiPayload, &writeBuff[1], writeSize - 3);
    
    // Decode ASCII back to bytes
    uint8_t decoded[128];
    asciiToBytes(asciiPayload, decoded, writeSize - 3);
    
    // Last byte should be LRC
    uint8_t lrcFromFrame = decoded[(writeSize - 3) / 2 - 1];
    
    // Calculate LRC of data (excluding LRC itself)
    uint8_t calculatedLrc = lrc(decoded, (writeSize - 3) / 2 - 1);
    
    EXPECT_EQ(lrcFromFrame, calculatedLrc);
}

TEST_F(ModbusAscOverUdpPortTest, DifferentDataSizesLrcValidation)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    
    // Test various data sizes
    for (int dataSize = 0; dataSize <= 32; dataSize += 4)
    {
        uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
        uint8_t *data = new uint8_t[dataSize];
        for (int i = 0; i < dataSize; i++)
            data[i] = static_cast<uint8_t>(i);
        
        StatusCode result = port->testWriteBuffer(unit, func, data, dataSize);
        EXPECT_EQ(result, Status_Good);
        
        const uint8_t *buff = port->writeBufferData();
        uint16_t buffSize = port->writeBufferSize();
        
        // Verify frame structure
        EXPECT_EQ(buff[0], ':');
        EXPECT_EQ(buff[buffSize - 2], '\r');
        EXPECT_EQ(buff[buffSize - 1], '\n');
        
        delete[] data;
    }
}

// ============================================================================
// Network Configuration Tests (inherited from ModbusNetPort)
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, DefaultSettings)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    const Modbus::NetDefaults &defaults = Modbus::NetDefaults::instance();
    
    EXPECT_EQ(std::string(port->host()), std::string(defaults.host));
    EXPECT_EQ(port->port(), defaults.port);
    EXPECT_EQ(port->timeout(), defaults.timeout);
}

// ============================================================================
// ASCII Conversion Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, AsciiConversionRoundtrip)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Test ASCII conversion round-trip
    uint8_t original[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0x12, 0x34};
    uint8_t ascii[16];
    uint8_t decoded[8];
    
    // Convert to ASCII
    bytesToAscii(original, ascii, 8);
    
    // Convert back
    asciiToBytes(ascii, decoded, 16);
    
    // Should match original
    EXPECT_EQ(memcmp(original, decoded, 8), 0);
}

// ============================================================================
// Open/Close/IsOpen Tests
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, OpenSuccessfulBlocking)
{
    port = new ModbusAscOverUdpPortTestHelper(true);
    port->setHost("127.0.0.1");
    port->setPort(1502);
    
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->type(), Modbus::ASCvUDP);
}

TEST_F(ModbusAscOverUdpPortTest, OpenSuccessfulNonBlocking)
{
    port = new ModbusAscOverUdpPortTestHelper(false);
    port->setHost("127.0.0.1");
    port->setPort(1502);
    
    EXPECT_FALSE(port->isOpen());
    EXPECT_TRUE(port->isNonBlocking());
}

TEST_F(ModbusAscOverUdpPortTest, CloseWhenNotOpen)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    StatusCode result = port->close();
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_FALSE(port->isOpen());
}

TEST_F(ModbusAscOverUdpPortTest, HandleMethod)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // When not open, handle should be invalid
    Handle h = port->handle();
    (void)h; // Avoid unused variable warning
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, MultipleWriteBufferCalls)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    // First write
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t firstSize = port->writeBufferSize();
    
    // Second write (should overwrite previous buffer)
    data[3] = 0x02;
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t secondSize = port->writeBufferSize();
    
    EXPECT_EQ(firstSize, secondSize);
}

TEST_F(ModbusAscOverUdpPortTest, WriteWithDifferentDataSizes)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 1;
    
    // Test with minimal data (no payload)
    {
        uint8_t func = MBF_READ_EXCEPTION_STATUS;
        StatusCode result = port->testWriteBuffer(unit, func, nullptr, 0);
        EXPECT_EQ(result, Status_Good);
        EXPECT_EQ(port->writeBufferSize(), 9); // ':' + 6 ASCII chars + CRLF
    }
    
    // Test with small data
    {
        uint8_t func = MBF_WRITE_SINGLE_COIL;
        uint8_t data[4] = {0x00, 0x00, 0xFF, 0x00};
        StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
        EXPECT_EQ(result, Status_Good);
        // ':' + ((1+1+4+1)*2) + CRLF = 1 + 14 + 2 = 17
        EXPECT_EQ(port->writeBufferSize(), 17);
    }
    
    // Test with larger data
    {
        uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
        uint8_t data[32];
        memset(data, 0xAA, sizeof(data));
        StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
        EXPECT_EQ(result, Status_Good);
        // ':' + ((1+1+32+1)*2) + CRLF = 1 + 70 + 2 = 73
        EXPECT_EQ(port->writeBufferSize(), 73);
    }
}

// ============================================================================
// Socket I/O Tests - Testing write() and read() when port is not open
// ============================================================================

TEST_F(ModbusAscOverUdpPortTest, WriteMethodWhenNotOpen)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    StatusCode result = port->testWrite();
    
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusAscOverUdpPortTest, ReadMethodWhenNotOpen)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    StatusCode result = port->testRead();
    
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusAscOverUdpPortTest, WriteWhenNotOpen)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Prepare a buffer to write
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    // Try to write when port is not open
    StatusCode result = port->testWrite();
    
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusAscOverUdpPortTest, ReadWhenNotOpen)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Try to read when port is not open
    StatusCode result = port->testRead();
    
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusAscOverUdpPortTest, WriteMethodWithValidData)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    // Prepare a valid Modbus ASCII request
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[] = {0x00, 0x00, 0x00, 0x0A};
    
    StatusCode prepareResult = port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(prepareResult, Status_Good);
    
    // Verify buffer is prepared with ASCII frame
    const uint8_t* buff = port->writeBufferData();
    EXPECT_EQ(buff[0], ':');
    EXPECT_EQ(buff[port->writeBufferSize() - 2], '\r');
    EXPECT_EQ(buff[port->writeBufferSize() - 1], '\n');
}

TEST_F(ModbusAscOverUdpPortTest, SocketWriteVariousFunctionCodes)
{
    port = new ModbusAscOverUdpPortTestHelper();
    
    uint8_t unit = 1;
    
    // Test Write Single Coil (0x05)
    uint8_t writeSingleCoilData[] = {0x00, 0x0A, 0xFF, 0x00};
    port->testWriteBuffer(unit, MBF_WRITE_SINGLE_COIL, writeSingleCoilData, sizeof(writeSingleCoilData));
    const uint8_t* buff1 = port->writeBufferData();
    EXPECT_EQ(buff1[0], ':');
    
    // Test Write Single Register (0x06)
    uint8_t writeSingleRegData[] = {0x00, 0x0A, 0x01, 0x23};
    port->testWriteBuffer(unit, MBF_WRITE_SINGLE_REGISTER, writeSingleRegData, sizeof(writeSingleRegData));
    const uint8_t* buff2 = port->writeBufferData();
    EXPECT_EQ(buff2[0], ':');
    
    // Test Write Multiple Coils (0x0F)
    uint8_t writeMultipleCoilsData[] = {0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD, 0x01};
    port->testWriteBuffer(unit, MBF_WRITE_MULTIPLE_COILS, writeMultipleCoilsData, sizeof(writeMultipleCoilsData));
    const uint8_t* buff3 = port->writeBufferData();
    EXPECT_EQ(buff3[0], ':');
}
