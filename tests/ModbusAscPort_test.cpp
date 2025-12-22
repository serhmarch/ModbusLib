#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusAscPort.h>
#include <ModbusPort_p.h>
#include <ModbusSerialPort_p.h>
#include <ModbusGlobal.h>

using namespace Modbus;

// Helper class to access protected members for testing
class ModbusAscPortTestHelper : public ModbusAscPort
{
public:
    ModbusAscPortTestHelper(bool blocking = true) : ModbusAscPort(blocking) {}

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
        ModbusSerialPortPrivate *d = d_ModbusSerialPort(this->d_ptr);
        memcpy(d->buff, data, size);
        d->sz = size;
    }

    const uint8_t* getInternalBuffer() const
    {
        ModbusSerialPortPrivate *d = d_ModbusSerialPort(this->d_ptr);
        return d->buff;
    }

    uint16_t getInternalBufferSize() const
    {
        ModbusSerialPortPrivate *d = d_ModbusSerialPort(this->d_ptr);
        return d->sz;
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

// Test Fixture for ModbusAscPort
class ModbusAscPortTest : public ::testing::Test
{
protected:
    ModbusAscPortTestHelper *port;

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

TEST_F(ModbusAscPortTest, TypeReturnsAsc)
{
    port = new ModbusAscPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::ASC);
}

TEST_F(ModbusAscPortTest, InitializationDefault)
{
    port = new ModbusAscPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::ASC);
    EXPECT_FALSE(port->isOpen());
    EXPECT_TRUE(port->isBlocking());
}

TEST_F(ModbusAscPortTest, InitializationBlockingMode)
{
    ModbusAscPortTestHelper *portBlocking = new ModbusAscPortTestHelper(true);
    EXPECT_TRUE(portBlocking->isBlocking());
    delete portBlocking;

    ModbusAscPortTestHelper *portNonBlocking = new ModbusAscPortTestHelper(false);
    EXPECT_FALSE(portNonBlocking->isBlocking());
    EXPECT_TRUE(portNonBlocking->isNonBlocking());
    delete portNonBlocking;
}

// ============================================================================
// `open()`-method tests
// ============================================================================

TEST_F(ModbusAscPortTest, OpenMethodClearsChangedFlag)
{
    auto *ascPort = new ModbusAscPort(true);
    
    ascPort->setPortName("somethingnonexisting.plc");
    EXPECT_TRUE(ascPort->isChanged());    
    auto status = ascPort->open();
    EXPECT_TRUE(Modbus::StatusIsBad(status));
    EXPECT_FALSE(ascPort->isChanged());    
    delete ascPort;
}

// ============================================================================
// Write Buffer Tests (ASCII Frame Construction with LRC)
// ============================================================================

TEST_F(ModbusAscPortTest, WriteBufferConstructsAsciiFrameAndReturnsGood)
{
    port = new ModbusAscPortTestHelper();
    
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
    
    // ASCII payload between ':' and CRLF
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

TEST_F(ModbusAscPortTest, WriteBufferReadHoldingRegisters)
{
    port = new ModbusAscPortTestHelper();
    
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

TEST_F(ModbusAscPortTest, WriteBufferWriteSingleCoil)
{
    port = new ModbusAscPortTestHelper();
    
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

TEST_F(ModbusAscPortTest, WriteBufferZeroLengthData)
{
    port = new ModbusAscPortTestHelper();
    
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

TEST_F(ModbusAscPortTest, WriteBufferLargeData)
{
    port = new ModbusAscPortTestHelper();
    
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

// ============================================================================
// Read Buffer Tests (ASCII Frame Parsing with LRC Validation)
// ============================================================================

TEST_F(ModbusAscPortTest, ReadBufferValidFrame)
{
    port = new ModbusAscPortTestHelper();
    
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
    ibuff[7] = lrcValue;  // LRC goes in position 7
    
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

TEST_F(ModbusAscPortTest, ReadBufferReadHoldingRegistersResponse)
{
    port = new ModbusAscPortTestHelper();
    
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
    uint8_t lrcValue = lrc(ibuff, 7);  // LRC on first 7 bytes
    ibuff[7] = lrcValue;
    
    // Convert to ASCII
    uint8_t asciiPayload[16];  // 8 bytes * 2 = 16 ASCII chars
    bytesToAscii(ibuff, asciiPayload, 8);
    
    // Build frame
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
    EXPECT_EQ(outBuff[0], byteCount);
}

TEST_F(ModbusAscPortTest, ReadBufferExceptionResponse)
{
    port = new ModbusAscPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = 0x83; // Exception
    uint8_t exceptionCode = 0x02;
    
    // Build internal buffer
    uint8_t ibuff[3];
    ibuff[0] = unit;
    ibuff[1] = func;
    ibuff[2] = exceptionCode;
    uint8_t lrcValue = lrc(ibuff, 3);
    ibuff[2] = lrcValue; // LRC replaces exception code position
    
    // Actually, for exception we need unit+func+exceptionCode+LRC
    uint8_t ibuff2[4];
    ibuff2[0] = unit;
    ibuff2[1] = func;
    ibuff2[2] = exceptionCode;
    lrcValue = lrc(ibuff2, 3);
    ibuff2[3] = lrcValue;
    
    // Convert to ASCII
    uint8_t asciiPayload[8];
    bytesToAscii(ibuff2, asciiPayload, 4);
    
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

TEST_F(ModbusAscPortTest, ReadBufferTooSmallFrame)
{
    port = new ModbusAscPortTestHelper();
    
    // Frame too small (just ':')
    uint8_t tooSmallFrame[1] = {':'};
    
    port->setInternalBuffer(tooSmallFrame, 1);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
}

TEST_F(ModbusAscPortTest, ReadBufferMissingColonRaises)
{
    port = new ModbusAscPortTestHelper();
    
    // Build frame with wrong start symbol
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[2] = {0x00, 0x01};
    
    uint8_t ibuff[4];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t lrcValue = lrc(ibuff, 4);
    ibuff[3] = lrcValue;
    
    uint8_t asciiPayload[8];
    bytesToAscii(ibuff, asciiPayload, 4);
    
    uint8_t frame[11];
    frame[0] = '#'; // Wrong start symbol
    memcpy(&frame[1], asciiPayload, 8);
    frame[9] = '\r';
    frame[10] = '\n';
    
    port->setInternalBuffer(frame, 11);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadAscMissColon);
}

TEST_F(ModbusAscPortTest, ReadBufferMissingCrlfRaises)
{
    port = new ModbusAscPortTestHelper();
    
    // Build frame without CRLF
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[2] = {0x00, 0x01};
    
    uint8_t ibuff[4];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    uint8_t lrcValue = lrc(ibuff, 4);
    ibuff[3] = lrcValue;
    
    uint8_t asciiPayload[8];
    bytesToAscii(ibuff, asciiPayload, 4);
    
    uint8_t frame[9];
    frame[0] = ':';
    memcpy(&frame[1], asciiPayload, 8);
    // Missing CRLF
    
    port->setInternalBuffer(frame, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadAscMissCrLf);
}

TEST_F(ModbusAscPortTest, ReadBufferWrongLrcRaises)
{
    port = new ModbusAscPortTestHelper();
    
    // Build frame with incorrect LRC
    uint8_t unit = 0x01;
    uint8_t func = 0x03;
    uint8_t data[2] = {0x00, 0x01};
    
    uint8_t ibuff[4];
    ibuff[0] = unit;
    ibuff[1] = func;
    memcpy(&ibuff[2], data, sizeof(data));
    ibuff[3] = 0x00; // Wrong LRC (should be calculated)
    
    uint8_t asciiPayload[8];
    bytesToAscii(ibuff, asciiPayload, 4);
    
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
    
    EXPECT_EQ(result, Status_BadLrc);
}

TEST_F(ModbusAscPortTest, ReadBufferMinimalValidFrame)
{
    port = new ModbusAscPortTestHelper();
    
    // Minimal frame: unit + func + LRC
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

TEST_F(ModbusAscPortTest, ReadBufferInvalidAsciiCharacters)
{
    port = new ModbusAscPortTestHelper();
    
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

TEST_F(ModbusAscPortTest, CompleteRequestResponseCycle)
{
    port = new ModbusAscPortTestHelper();
    
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
    uint8_t ibuff[7];
    ibuff[0] = unit;
    ibuff[1] = func;
    ibuff[2] = 0x04; // Byte count
    ibuff[3] = 0x00;
    ibuff[4] = 0x0A;
    ibuff[5] = 0x00;
    ibuff[6] = 0x14;
    uint8_t lrcValue = lrc(ibuff, 7);
    ibuff[6] = lrcValue; // Replace last byte with LRC
    
    // Actually need to include all data + LRC
    uint8_t ibuff2[8];
    ibuff2[0] = unit;
    ibuff2[1] = func;
    ibuff2[2] = 0x04;
    ibuff2[3] = 0x00;
    ibuff2[4] = 0x0A;
    ibuff2[5] = 0x00;
    ibuff2[6] = 0x14;
    lrcValue = lrc(ibuff2, 7);
    ibuff2[7] = lrcValue;
    
    uint8_t asciiPayload[16];
    bytesToAscii(ibuff2, asciiPayload, 8);
    
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
}

// ============================================================================
// Serial Port Configuration Tests
// ============================================================================

TEST_F(ModbusAscPortTest, SerialPortConfiguration)
{
    port = new ModbusAscPortTestHelper();
    
    // Test port name configuration
    port->setPortName("COM3");
    EXPECT_EQ(std::string(port->portName()), std::string("COM3"));
    
    // Test baud rate
    port->setBaudRate(9600);
    EXPECT_EQ(port->baudRate(), 9600);
    
    // Test data bits
    port->setDataBits(7); // ASCII typically uses 7 data bits
    EXPECT_EQ(port->dataBits(), 7);
    
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

TEST_F(ModbusAscPortTest, DefaultSerialSettings)
{
    port = new ModbusAscPortTestHelper();
    
    const Modbus::Defaults &defaults = Modbus::Defaults::instance();
    
    EXPECT_EQ(port->baudRate(), defaults.baudRate);
    EXPECT_EQ(port->dataBits(), defaults.dataBits);
    EXPECT_EQ(port->parity(), defaults.parity);
    EXPECT_EQ(port->stopBits(), defaults.stopBits);
    EXPECT_EQ(port->flowControl(), defaults.flowControl);
}

// ============================================================================
// LRC Validation Tests
// ============================================================================

TEST_F(ModbusAscPortTest, LrcCalculationConsistency)
{
    port = new ModbusAscPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_READ_COILS;
    uint8_t data[4] = {0x00, 0x13, 0x00, 0x25};
    
    // Write buffer
    port->testWriteBuffer(unit, func, data, sizeof(data));
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    
    // Extract ASCII payload
    uint8_t asciiPayload[256];
    memcpy(asciiPayload, &writeBuff[1], writeSize - 3); // Skip ':' and CRLF
    
    // Decode ASCII back to bytes
    uint8_t decoded[128];
    asciiToBytes(asciiPayload, decoded, writeSize - 3);
    
    // Last byte should be LRC
    uint8_t lrcFromFrame = decoded[(writeSize - 3) / 2 - 1];
    
    // Calculate LRC of data (excluding LRC itself)
    uint8_t calculatedLrc = lrc(decoded, (writeSize - 3) / 2 - 1);
    
    EXPECT_EQ(lrcFromFrame, calculatedLrc);
}

TEST_F(ModbusAscPortTest, DifferentDataSizesLrcValidation)
{
    port = new ModbusAscPortTestHelper();
    
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
// Edge Cases and Error Handling
// ============================================================================

TEST_F(ModbusAscPortTest, BufferOverflow)
{
    port = new ModbusAscPortTestHelper();
    
    uint8_t unit = 0x01;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[MB_ASC_IO_BUFF_SZ]; // Too large
    memset(data, 0, sizeof(data));
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_BadWriteBufferOverflow);
}

TEST_F(ModbusAscPortTest, WriteReadWhenNotOpen)
{
    port = new ModbusAscPortTestHelper();
    
    // Port is not open
    EXPECT_FALSE(port->isOpen());
    
    // Try to write
    StatusCode writeResult = port->testWrite();
    EXPECT_NE(writeResult, Status_Good);
    
    // Try to read
    StatusCode readResult = port->testRead();
    EXPECT_NE(readResult, Status_Good);
}

TEST_F(ModbusAscPortTest, MultipleWriteBufferCalls)
{
    port = new ModbusAscPortTestHelper();
    
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

TEST_F(ModbusAscPortTest, CloseWhenNotOpen)
{
    port = new ModbusAscPortTestHelper();
    
    StatusCode result = port->close();
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_FALSE(port->isOpen());
}

TEST_F(ModbusAscPortTest, HandleMethod)
{
    port = new ModbusAscPortTestHelper();
    
    // When not open, handle should be invalid
    Handle h = port->handle();
    (void)h; // Avoid unused variable warning
}

TEST_F(ModbusAscPortTest, AsciiConversionRoundtrip)
{
    port = new ModbusAscPortTestHelper();
    
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
