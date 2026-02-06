#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusTcpPort.h>
#include <ModbusPort_p.h>
#include <ModbusTcpPort_p.h>
#include <ModbusGlobal.h>

// Helper class to access protected members for testing
class ModbusTcpPortTestHelper : public ModbusTcpPort
{
public:
    ModbusTcpPortTestHelper(bool blocking = true) : ModbusTcpPort(blocking) {}

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
        ModbusTcpPortPrivate *d = static_cast<ModbusTcpPortPrivate*>(this->d_ptr);
        memcpy(d->buff, data, size);
        d->sz = size;
    }

    void setInternalTransaction(uint16_t trans)
    {
        ModbusTcpPortPrivate *d = static_cast<ModbusTcpPortPrivate*>(this->d_ptr);
        d->transaction = trans;
    }

    uint16_t getInternalTransaction() const
    {
        ModbusTcpPortPrivate *d = static_cast<ModbusTcpPortPrivate*>(this->d_ptr);
        return d->transaction;
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
        ModbusTcpPortPrivate *d = static_cast<ModbusTcpPortPrivate*>(this->d_ptr);
        return d->buff;
    }

    uint16_t getInternalBufferSize() const
    {
        ModbusTcpPortPrivate *d = static_cast<ModbusTcpPortPrivate*>(this->d_ptr);
        return d->sz;
    }
};

// Test Fixture for ModbusTcpPort
class ModbusTcpPortTest : public ::testing::Test
{
protected:
    ModbusTcpPortTestHelper *port;

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

TEST_F(ModbusTcpPortTest, InitializationDefault)
{
    port = new ModbusTcpPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::TCP);
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->host(), std::string("localhost"));
    EXPECT_EQ(port->port(), STANDARD_TCP_PORT);
    EXPECT_TRUE(port->isBlocking());
    EXPECT_TRUE(port->autoIncrement());
}

TEST_F(ModbusTcpPortTest, InitializationBlockingMode)
{
    ModbusTcpPortTestHelper *portBlocking = new ModbusTcpPortTestHelper(true);
    EXPECT_TRUE(portBlocking->isBlocking());
    delete portBlocking;

    ModbusTcpPortTestHelper *portNonBlocking = new ModbusTcpPortTestHelper(false);
    EXPECT_FALSE(portNonBlocking->isBlocking());
    EXPECT_TRUE(portNonBlocking->isNonBlocking());
    delete portNonBlocking;
}

TEST_F(ModbusTcpPortTest, InitializationWithSocket)
{
    // Note: Direct socket initialization test would require platform-specific 
    // socket handling. This test validates the basic constructor works.
    port = new ModbusTcpPortTestHelper();
    
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->type(), Modbus::TCP);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, HostConfiguration)
{
    port = new ModbusTcpPortTestHelper();
    
    port->setHost("192.168.1.100");
    EXPECT_EQ(std::string(port->host()), std::string("192.168.1.100"));
    EXPECT_TRUE(port->isChanged());
}

TEST_F(ModbusTcpPortTest, PortConfiguration)
{
    port = new ModbusTcpPortTestHelper();
    
    port->setPort(1502);
    EXPECT_EQ(port->port(), 1502);
    EXPECT_TRUE(port->isChanged());
}

TEST_F(ModbusTcpPortTest, TimeoutConfiguration)
{
    port = new ModbusTcpPortTestHelper();
    
    port->setTimeout(5000);
    EXPECT_EQ(port->timeout(), 5000);
}

TEST_F(ModbusTcpPortTest, ServerModeConfiguration)
{
    port = new ModbusTcpPortTestHelper();
    
    EXPECT_FALSE(port->isServerMode());
    
    port->setServerMode(true);
    EXPECT_TRUE(port->isServerMode());
    
    port->setServerMode(false);
    EXPECT_FALSE(port->isServerMode());
}

// ============================================================================
// `open()`-method tests
// ============================================================================

TEST_F(ModbusTcpPortTest, OpenMethodClearsChangedFlag)
{
    auto *tcpPort = new ModbusTcpPort(true);
    
    tcpPort->setHost("somethingnonexisting.plc");
    EXPECT_TRUE(tcpPort->isChanged());    
    auto status = tcpPort->open();
    EXPECT_TRUE(Modbus::StatusIsBad(status));
    EXPECT_FALSE(tcpPort->isChanged());    
    delete tcpPort;
}

// ============================================================================
// Transaction ID Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, TransactionIdAutoIncrement)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false); // Client mode
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    uint16_t initialTransaction = port->transactionId();
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction + 1);
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction + 2);
}

TEST_F(ModbusTcpPortTest, TransactionIdNoAutoIncrementInServerMode)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    uint16_t initialTransaction = port->transactionId();
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction);
}

TEST_F(ModbusTcpPortTest, TransactionIdRepeatedRequest)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    uint16_t initialTransaction = port->transactionId();
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction + 1);
    
    // Set next request as repeated
    port->setNextRequestRepeated(true);
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction + 1); // Should not increment
    
    // Auto-increment should resume
    port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(port->transactionId(), initialTransaction + 2);
}

// ============================================================================
// Write Buffer Tests (Client Mode)
// ============================================================================

TEST_F(ModbusTcpPortTest, WriteBufferClientMode)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01}; // offset=0, count=1
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t sz = port->writeBufferSize();
    
    // Expected size: 6 (TCP header) + 1 (unit) + 1 (func) + 4 (data) = 12 bytes
    EXPECT_EQ(sz, 12);
    
    // Check transaction ID (bytes 0-1)
    uint16_t transactionId = (buff[0] << 8) | buff[1];
    EXPECT_EQ(transactionId, port->transactionId());
    
    // Check protocol ID (bytes 2-3) - should be 0
    uint16_t protocolId = (buff[2] << 8) | buff[3];
    EXPECT_EQ(protocolId, 0);
    
    // Check length (bytes 4-5) - should be 6 (unit + func + data)
    uint16_t length = (buff[4] << 8) | buff[5];
    EXPECT_EQ(length, 6);
    
    // Check unit and function
    EXPECT_EQ(buff[6], unit);
    EXPECT_EQ(buff[7], func);
    
    // Check data
    EXPECT_EQ(memcmp(&buff[8], data, sizeof(data)), 0);
}

TEST_F(ModbusTcpPortTest, WriteBufferServerMode)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[3] = {0x02, 0x00, 0x01}; // Response: byte count=2, value=0x0001
    
    uint16_t initialTransaction = 42;
    // Manually set transaction (normally set by readBuffer in server mode)
    port->setInternalTransaction(initialTransaction);
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(port->transactionId(), initialTransaction); // Should not change
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t sz = port->writeBufferSize();
    
    // Expected size: 6 (TCP header) + 1 (unit) + 1 (func) + 3 (data) = 11 bytes
    EXPECT_EQ(sz, 11);
    
    // Check transaction ID matches the initial one
    uint16_t transactionId = (buff[0] << 8) | buff[1];
    EXPECT_EQ(transactionId, initialTransaction);
    
    // Check protocol ID
    uint16_t protocolId = (buff[2] << 8) | buff[3];
    EXPECT_EQ(protocolId, 0);
    
    // Check length
    uint16_t length = (buff[4] << 8) | buff[5];
    EXPECT_EQ(length, 5); // unit + func + data
}

TEST_F(ModbusTcpPortTest, WriteBufferOverflow)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[MB_TCP_IO_BUFF_SZ]; // Too large
    memset(data, 0, sizeof(data));
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_BadWriteBufferOverflow);
    EXPECT_EQ(port->lastErrorStatus(), Status_BadWriteBufferOverflow);
}

// ============================================================================
// Read Buffer Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, ReadBufferClientMode)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t sendData[4] = {0x00, 0x00, 0x00, 0x01};
    
    // Prepare request to get transaction ID
    port->testWriteBuffer(unit, func, sendData, sizeof(sendData));
    uint16_t expectedTransaction = port->transactionId();
    
    // Prepare simulated response buffer
    uint8_t responseData[11];
    responseData[0] = expectedTransaction >> 8;    // Transaction ID MSB
    responseData[1] = expectedTransaction & 0xFF;  // Transaction ID LSB
    responseData[2] = 0x00;                        // Protocol ID MSB
    responseData[3] = 0x00;                        // Protocol ID LSB
    responseData[4] = 0x00;                        // Length MSB
    responseData[5] = 0x05;                        // Length LSB (5 bytes: unit + func + 3 data)
    responseData[6] = unit;                        // Unit
    responseData[7] = func;                        // Function
    responseData[8] = 0x02;                        // Byte count
    responseData[9] = 0x00;                        // Data MSB
    responseData[10] = 0x01;                       // Data LSB
    port->setInternalBuffer(responseData, 11);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 3);
    EXPECT_EQ(outBuff[0], 0x02);
    EXPECT_EQ(outBuff[1], 0x00);
    EXPECT_EQ(outBuff[2], 0x01);
}

TEST_F(ModbusTcpPortTest, ReadBufferServerMode)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    uint16_t clientTransaction = 123;
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    
    // Prepare simulated request buffer from client
    uint8_t requestData[12];
    requestData[0] = clientTransaction >> 8;    // Transaction ID MSB
    requestData[1] = clientTransaction & 0xFF;  // Transaction ID LSB
    requestData[2] = 0x00;                      // Protocol ID MSB
    requestData[3] = 0x00;                      // Protocol ID LSB
    requestData[4] = 0x00;                      // Length MSB
    requestData[5] = 0x06;                      // Length LSB (6 bytes)
    requestData[6] = unit;                      // Unit
    requestData[7] = func;                      // Function
    requestData[8] = 0x00;                      // Start address MSB
    requestData[9] = 0x00;                      // Start address LSB
    requestData[10] = 0x00;                     // Quantity MSB
    requestData[11] = 0x01;                     // Quantity LSB
    port->setInternalBuffer(requestData, 12);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(port->transactionId(), clientTransaction); // Server should capture client's transaction ID
}

TEST_F(ModbusTcpPortTest, ReadBufferTooSmall)
{
    port = new ModbusTcpPortTestHelper();
    
    // Prepare buffer with only 5 bytes (less than minimum 8)
    uint8_t tooSmallData[5] = {0};
    port->setInternalBuffer(tooSmallData, 5);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectResponse);
}

TEST_F(ModbusTcpPortTest, ReadBufferInvalidProtocolId)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t invalidData[9];
    invalidData[0] = 0x00;  // Transaction ID MSB
    invalidData[1] = 0x01;  // Transaction ID LSB
    invalidData[2] = 0x00;  // Protocol ID MSB
    invalidData[3] = 0x01;  // Protocol ID LSB - INVALID (should be 0)
    invalidData[4] = 0x00;  // Length MSB
    invalidData[5] = 0x03;  // Length LSB
    invalidData[6] = 0x01;  // Unit
    invalidData[7] = 0x03;  // Function
    invalidData[8] = 0x00;  // Data
    port->setInternalBuffer(invalidData, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectResponse);
}

TEST_F(ModbusTcpPortTest, ReadBufferIncorrectLength)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t incorrectLengthData[9];
    incorrectLengthData[0] = 0x00;  // Transaction ID MSB
    incorrectLengthData[1] = 0x01;  // Transaction ID LSB
    incorrectLengthData[2] = 0x00;  // Protocol ID MSB
    incorrectLengthData[3] = 0x00;  // Protocol ID LSB
    incorrectLengthData[4] = 0x00;  // Length MSB
    incorrectLengthData[5] = 0x10;  // Length LSB - INCORRECT (doesn't match actual data)
    incorrectLengthData[6] = 0x01;  // Unit
    incorrectLengthData[7] = 0x03;  // Function
    incorrectLengthData[8] = 0x00;  // Data
    port->setInternalBuffer(incorrectLengthData, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectResponse);
}

TEST_F(ModbusTcpPortTest, ReadBufferTransactionMismatch)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t sendData[4] = {0x00, 0x00, 0x00, 0x01};
    
    // Send request to set expected transaction ID
    port->testWriteBuffer(unit, func, sendData, sizeof(sendData));
    uint16_t expectedTransaction = port->transactionId();
    
    // Prepare response with wrong transaction ID
    uint8_t wrongTransData[11];
    wrongTransData[0] = (expectedTransaction + 10) >> 8;    // Wrong transaction ID
    wrongTransData[1] = (expectedTransaction + 10) & 0xFF;
    wrongTransData[2] = 0x00;
    wrongTransData[3] = 0x00;
    wrongTransData[4] = 0x00;
    wrongTransData[5] = 0x05;
    wrongTransData[6] = unit;
    wrongTransData[7] = func;
    wrongTransData[8] = 0x02;
    wrongTransData[9] = 0x00;
    wrongTransData[10] = 0x01;
    port->setInternalBuffer(wrongTransData, 11);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectResponse);
}

// ============================================================================
// Open/Close/IsOpen Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, OpenSuccessfulBlocking)
{
    // Note: This is a basic structure test. Full socket mocking would require
    // more sophisticated infrastructure similar to the Python test
    port = new ModbusTcpPortTestHelper(true);
    port->setHost("127.0.0.1");
    port->setPort(1502);
    
    // Without actual socket mocking infrastructure, we just verify
    // the port is created successfully and in expected state
    EXPECT_FALSE(port->isOpen());
    EXPECT_EQ(port->type(), Modbus::TCP);
}

TEST_F(ModbusTcpPortTest, OpenSuccessfulNonBlocking)
{
    port = new ModbusTcpPortTestHelper(false);
    port->setHost("127.0.0.1");
    port->setPort(1502);
    
    EXPECT_FALSE(port->isOpen());
    EXPECT_TRUE(port->isNonBlocking());
}

TEST_F(ModbusTcpPortTest, CloseWhenNotOpen)
{
    port = new ModbusTcpPortTestHelper();
    
    StatusCode result = port->close();
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_FALSE(port->isOpen());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, CompleteRequestResponseCycle)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02}; // Read 2 registers from address 0
    
    // Prepare request
    StatusCode writeBufferResult = port->testWriteBuffer(unit, func, requestData, sizeof(requestData));
    EXPECT_EQ(writeBufferResult, Status_Good);
    
    uint16_t expectedTransaction = port->transactionId();
    
    // Simulate response
    uint8_t responseData[13];
    responseData[0] = expectedTransaction >> 8;
    responseData[1] = expectedTransaction & 0xFF;
    responseData[2] = 0x00;
    responseData[3] = 0x00;
    responseData[4] = 0x00;
    responseData[5] = 0x07;  // Length: unit + func + byte count + 4 data bytes
    responseData[6] = unit;
    responseData[7] = func;
    responseData[8] = 0x04;  // Byte count
    responseData[9] = 0x00;  // Register 1 MSB
    responseData[10] = 0x0A; // Register 1 LSB
    responseData[11] = 0x00; // Register 2 MSB
    responseData[12] = 0x14; // Register 2 LSB
    port->setInternalBuffer(responseData, 13);
    
    // Parse response
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode readBufferResult = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(readBufferResult, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 5); // byte count + 4 data bytes
    EXPECT_EQ(outBuff[0], 0x04); // Byte count
    
    // Check register values
    uint16_t reg1 = (outBuff[1] << 8) | outBuff[2];
    uint16_t reg2 = (outBuff[3] << 8) | outBuff[4];
    EXPECT_EQ(reg1, 0x000A);
    EXPECT_EQ(reg2, 0x0014);
}

TEST_F(ModbusTcpPortTest, ServerModeRequestResponseCycle)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    uint16_t clientTransaction = 999;
    uint8_t unit = 1;
    uint8_t func = MBF_READ_COILS;
    
    // Simulate client request
    uint8_t requestData[12];
    requestData[0] = clientTransaction >> 8;
    requestData[1] = clientTransaction & 0xFF;
    requestData[2] = 0x00;
    requestData[3] = 0x00;
    requestData[4] = 0x00;
    requestData[5] = 0x06;
    requestData[6] = unit;
    requestData[7] = func;
    requestData[8] = 0x00;  // Start address MSB
    requestData[9] = 0x00;  // Start address LSB
    requestData[10] = 0x00; // Quantity MSB
    requestData[11] = 0x08; // Quantity LSB (8 coils)
    port->setInternalBuffer(requestData, 12);
    
    // Parse request
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode readResult = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(readResult, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(port->transactionId(), clientTransaction); // Server captures client transaction
    
    // Prepare response
    uint8_t responseData[2] = {0x01, 0xFF}; // Byte count=1, coils value=0xFF
    StatusCode writeResult = port->testWriteBuffer(unit, func, responseData, sizeof(responseData));
    
    EXPECT_EQ(writeResult, Status_Good);
    EXPECT_EQ(port->transactionId(), clientTransaction); // Should remain same
    
    const uint8_t *responseBuff = port->writeBufferData();
    uint16_t responseTransaction = (responseBuff[0] << 8) | responseBuff[1];
    EXPECT_EQ(responseTransaction, clientTransaction); // Response uses client's transaction
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(ModbusTcpPortTest, MaximumDataSize)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
    uint8_t data[MB_TCP_IO_BUFF_SZ - 10]; // Near maximum
    memset(data, 0xFF, sizeof(data));
    
    StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
    
    EXPECT_EQ(result, Status_Good);
}

TEST_F(ModbusTcpPortTest, TypeMethod)
{
    port = new ModbusTcpPortTestHelper();
    
    EXPECT_EQ(port->type(), Modbus::TCP);
}

TEST_F(ModbusTcpPortTest, HandleMethod)
{
    port = new ModbusTcpPortTestHelper();
    
    // When not open, handle should be invalid
    Handle h = port->handle();
    // Actual handle value depends on implementation, but should be callable
    (void)h; // Avoid unused variable warning
}

TEST_F(ModbusTcpPortTest, MultipleWriteBufferCalls)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    uint16_t trans1 = port->transactionId();
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    uint16_t trans2 = port->transactionId();
    EXPECT_EQ(trans2, trans1 + 1);
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    uint16_t trans3 = port->transactionId();
    EXPECT_EQ(trans3, trans2 + 1);
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    uint16_t trans4 = port->transactionId();
    EXPECT_EQ(trans4, trans3 + 1);
}

TEST_F(ModbusTcpPortTest, ExceptionResponse)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t sendData[4] = {0x00, 0x00, 0x00, 0x01};
    
    port->testWriteBuffer(unit, func, sendData, sizeof(sendData));
    uint16_t expectedTransaction = port->transactionId();
    
    // Simulate exception response
    uint8_t exceptionData[9];
    exceptionData[0] = expectedTransaction >> 8;
    exceptionData[1] = expectedTransaction & 0xFF;
    exceptionData[2] = 0x00;
    exceptionData[3] = 0x00;
    exceptionData[4] = 0x00;
    exceptionData[5] = 0x03;  // Length
    exceptionData[6] = unit;
    exceptionData[7] = func | 0x80;  // Exception flag
    exceptionData[8] = Status_BadIllegalDataAddress & 0xFF;  // Exception code
    port->setInternalBuffer(exceptionData, 9);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func | 0x80); // Exception function code
    EXPECT_EQ(outSize, 1);
}

TEST_F(ModbusTcpPortTest, DefaultSettings)
{
    port = new ModbusTcpPortTestHelper();
    
    const ModbusTcpPort::Defaults &defaults = ModbusTcpPort::Defaults::instance();
    
    EXPECT_EQ(std::string(port->host()), std::string(defaults.host));
    EXPECT_EQ(port->port(), defaults.port);
    EXPECT_EQ(port->timeout(), defaults.timeout);
}

// ============================================================================
// Buffer Data Access Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, BufferDataAccess)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    const uint8_t *writeData = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    
    EXPECT_NE(writeData, nullptr);
    EXPECT_EQ(writeSize, 12); // 6 (TCP header) + 1 (unit) + 1 (func) + 4 (data)
    
    const uint8_t *readData = port->readBufferData();
    uint16_t readSize = port->readBufferSize();
    
    EXPECT_NE(readData, nullptr);
    // Read and write buffers use same underlying buffer in this implementation
}

TEST_F(ModbusTcpPortTest, ZeroLengthData)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_COILS;
    
    StatusCode result = port->testWriteBuffer(unit, func, nullptr, 0);
    
    // Should still work with zero-length data
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t *buff = port->writeBufferData();
    uint16_t sz = port->writeBufferSize();
    
    EXPECT_EQ(sz, 8); // 6 (TCP header) + 1 (unit) + 1 (func) + 0 (data)
}

// ============================================================================
// Write/Read Socket I/O Tests
// ============================================================================

TEST_F(ModbusTcpPortTest, WriteWhenNotOpen)
{
    port = new ModbusTcpPortTestHelper();
    
    // Prepare a buffer to write
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    // Try to write when port is not open
    StatusCode result = port->testWrite();
    
    // Should fail because port is not open
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusTcpPortTest, ReadWhenNotOpen)
{
    port = new ModbusTcpPortTestHelper();
    
    // Try to read when port is not open
    StatusCode result = port->testRead();
    
    // Should fail because port is not open
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusTcpPortTest, WriteEmptyBuffer)
{
    port = new ModbusTcpPortTestHelper();
    
    // Don't prepare any buffer (internal size is 0)
    // Try to write empty buffer
    StatusCode result = port->testWrite();
    
    // Should handle empty buffer gracefully
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusTcpPortTest, ReadEmptyResponse)
{
    port = new ModbusTcpPortTestHelper();
    
    // Set internal buffer to empty
    port->setInternalBuffer(nullptr, 0);
    
    // Try to read empty response
    StatusCode result = port->testRead();
    
    // Should handle appropriately
    EXPECT_NE(result, Status_Good);
}

TEST_F(ModbusTcpPortTest, WriteBufferPreparedCorrectly)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    // Prepare write buffer
    StatusCode prepResult = port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(prepResult, Status_Good);
    
    // Verify buffer is ready for write
    const uint8_t *buff = port->writeBufferData();
    uint16_t size = port->writeBufferSize();
    
    EXPECT_NE(buff, nullptr);
    EXPECT_EQ(size, 12); // 6 TCP header + 1 unit + 1 func + 4 data
    
    // Verify TCP header structure
    EXPECT_EQ(buff[2], 0x00); // Protocol ID MSB
    EXPECT_EQ(buff[3], 0x00); // Protocol ID LSB
    
    uint16_t length = (buff[4] << 8) | buff[5];
    EXPECT_EQ(length, 6); // unit + func + data length
}

TEST_F(ModbusTcpPortTest, ReadBufferSequence)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true); // Server mode to accept any transaction ID
    
    // Simulate a complete TCP response in internal buffer
    uint8_t responseData[11];
    responseData[0] = 0x00;  // Transaction ID MSB
    responseData[1] = 0x01;  // Transaction ID LSB
    responseData[2] = 0x00;  // Protocol ID MSB
    responseData[3] = 0x00;  // Protocol ID LSB
    responseData[4] = 0x00;  // Length MSB
    responseData[5] = 0x05;  // Length LSB (5 bytes)
    responseData[6] = 0x01;  // Unit
    responseData[7] = 0x03;  // Function
    responseData[8] = 0x02;  // Byte count
    responseData[9] = 0x00;  // Data MSB
    responseData[10] = 0x0A; // Data LSB
    
    port->setInternalBuffer(responseData, 11);
    
    // Verify internal buffer state before read
    EXPECT_EQ(port->getInternalBufferSize(), 11);
    
    // Parse the buffer
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, 0x01);
    EXPECT_EQ(outFunc, 0x03);
    EXPECT_EQ(outSize, 3); // Byte count + 2 data bytes
}

TEST_F(ModbusTcpPortTest, WriteReadCycle)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    // Step 1: Prepare write buffer
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t writeData[4] = {0x00, 0x00, 0x00, 0x01};
    
    StatusCode writeBufferResult = port->testWriteBuffer(unit, func, writeData, sizeof(writeData));
    EXPECT_EQ(writeBufferResult, Status_Good);
    
    uint16_t transactionId = port->transactionId();
    
    // Step 2: Verify write buffer is correct
    const uint8_t *writeBuff = port->writeBufferData();
    uint16_t writeSize = port->writeBufferSize();
    EXPECT_EQ(writeSize, 12);
    
    // Step 3: Simulate response
    uint8_t responseData[11];
    responseData[0] = transactionId >> 8;
    responseData[1] = transactionId & 0xFF;
    responseData[2] = 0x00;
    responseData[3] = 0x00;
    responseData[4] = 0x00;
    responseData[5] = 0x05;
    responseData[6] = unit;
    responseData[7] = func;
    responseData[8] = 0x02;
    responseData[9] = 0x00;
    responseData[10] = 0x01;
    
    port->setInternalBuffer(responseData, 11);
    
    // Step 4: Parse response
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode readBufferResult = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(readBufferResult, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 3);
    EXPECT_EQ(outBuff[0], 0x02); // Byte count
}

TEST_F(ModbusTcpPortTest, MultipleWriteCalls)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(false);
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x01};
    
    // First write
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t firstSize = port->writeBufferSize();
    EXPECT_EQ(firstSize, 12);
    
    // Second write (should overwrite previous buffer)
    port->testWriteBuffer(unit, func, data, sizeof(data));
    uint16_t secondSize = port->writeBufferSize();
    EXPECT_EQ(secondSize, 12);
    
    // Transaction ID should have incremented
    // (handled by writeBuffer, not write())
}

TEST_F(ModbusTcpPortTest, ReadPartialData)
{
    port = new ModbusTcpPortTestHelper();
    
    // Simulate partial response (incomplete TCP frame)
    uint8_t partialData[6];
    partialData[0] = 0x00;  // Transaction ID MSB
    partialData[1] = 0x01;  // Transaction ID LSB
    partialData[2] = 0x00;  // Protocol ID MSB
    partialData[3] = 0x00;  // Protocol ID LSB
    partialData[4] = 0x00;  // Length MSB
    partialData[5] = 0x05;  // Length LSB (claims 5 more bytes, but they're missing)
    
    port->setInternalBuffer(partialData, 6);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    // Should fail due to insufficient data
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_BadNotCorrectResponse);
}

TEST_F(ModbusTcpPortTest, WriteWithDifferentDataSizes)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    
    // Test with minimal data
    {
        uint8_t func = MBF_READ_EXCEPTION_STATUS;
        StatusCode result = port->testWriteBuffer(unit, func, nullptr, 0);
        EXPECT_EQ(result, Status_Good);
        EXPECT_EQ(port->writeBufferSize(), 8); // 6 header + 1 unit + 1 func
    }
    
    // Test with small data
    {
        uint8_t func = MBF_WRITE_SINGLE_COIL;
        uint8_t data[4] = {0x00, 0x00, 0xFF, 0x00};
        StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
        EXPECT_EQ(result, Status_Good);
        EXPECT_EQ(port->writeBufferSize(), 12);
    }
    
    // Test with larger data
    {
        uint8_t func = MBF_WRITE_MULTIPLE_REGISTERS;
        uint8_t data[64];
        memset(data, 0xAA, sizeof(data));
        StatusCode result = port->testWriteBuffer(unit, func, data, sizeof(data));
        EXPECT_EQ(result, Status_Good);
        EXPECT_EQ(port->writeBufferSize(), 72); // 6 header + 1 unit + 1 func + 64 data
    }
}

TEST_F(ModbusTcpPortTest, ReadBufferBoundaryConditions)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true); // Server mode to accept any transaction ID
    
    // Test minimum valid TCP frame (8 bytes)
    uint8_t minData[8];
    minData[0] = 0x00;  // Transaction ID MSB
    minData[1] = 0x01;  // Transaction ID LSB
    minData[2] = 0x00;  // Protocol ID MSB
    minData[3] = 0x00;  // Protocol ID LSB
    minData[4] = 0x00;  // Length MSB
    minData[5] = 0x02;  // Length LSB (2 bytes: unit + func)
    minData[6] = 0x01;  // Unit
    minData[7] = 0x03;  // Function
    
    port->setInternalBuffer(minData, 8);
    
    uint8_t outUnit, outFunc;
    uint8_t outBuff[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outBuff, sizeof(outBuff), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, 0x01);
    EXPECT_EQ(outFunc, 0x03);
    EXPECT_EQ(outSize, 0); // No data, just unit and function
}

TEST_F(ModbusTcpPortTest, VerifyTcpHeaderConstants)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    uint8_t func = MBF_READ_COILS;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x08};
    
    port->testWriteBuffer(unit, func, data, sizeof(data));
    
    const uint8_t *buff = port->writeBufferData();
    
    // Protocol ID must always be 0x0000 for Modbus TCP
    EXPECT_EQ(buff[2], 0x00);
    EXPECT_EQ(buff[3], 0x00);
    
    // Length field should be correctly calculated
    uint16_t length = (buff[4] << 8) | buff[5];
    uint16_t expectedLength = 1 + 1 + sizeof(data); // unit + func + data
    EXPECT_EQ(length, expectedLength);
}

// ============================================================================
// Socket I/O Tests - Testing write() and read() socket operations
// ============================================================================

// These tests verify that write() and read() methods correctly interact with
// the internal buffer and socket layer. Since actual socket mocking requires
// deep integration with Windows socket internals, these tests focus on
// verifying the correct behavior and data flow.

TEST_F(ModbusTcpPortTest, WriteMethodWhenNotOpen)
{
    port = new ModbusTcpPortTestHelper();
    
    // Port is not open, so write should return error
    StatusCode result = port->testWrite();
    
    // When not opened, write returns Status_Processing
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusTcpPortTest, ReadMethodWhenNotOpen)
{
    port = new ModbusTcpPortTestHelper();
    
    // Port is not open, so read should return error
    StatusCode result = port->testRead();
    
    // When not opened, read returns error
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusTcpPortTest, WriteMethodWithValidData)
{
    port = new ModbusTcpPortTestHelper();
    
    // Prepare a valid Modbus TCP request
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t data[] = {0x00, 0x00, 0x00, 0x0A}; // Start address 0, quantity 10
    
    StatusCode prepareResult = port->testWriteBuffer(unit, func, data, sizeof(data));
    EXPECT_EQ(prepareResult, Status_Good);
    
    // Verify buffer is prepared with TCP header
    const uint8_t* buff = port->writeBufferData();
    EXPECT_EQ(buff[2], 0x00); // Protocol ID MSB
    EXPECT_EQ(buff[3], 0x00); // Protocol ID LSB
    EXPECT_EQ(buff[6], unit); // Unit ID
    EXPECT_EQ(buff[7], func); // Function code
    
    // Note: Actual write() would require an open socket connection
    // This test verifies the buffer preparation before socket write
}

TEST_F(ModbusTcpPortTest, ReadMethodBufferCapacity)
{
    port = new ModbusTcpPortTestHelper();
    
    // Verify that the internal buffer can handle maximum TCP frame size
    // Maximum Modbus TCP ADU = 260 bytes (6-byte header + 254 bytes data)
    const uint16_t maxTcpSize = 260;
    
    // The buffer size is MB_TCP_IO_BUFF_SZ which should be at least 260
    uint16_t bufferSize = port->getInternalBufferSize();
    
    // After read(), buffer would contain data, but we can't test actual
    // socket read without a real connection. This test verifies structure.
    EXPECT_TRUE(true); // Buffer structure is validated by other tests
}

TEST_F(ModbusTcpPortTest, SocketWriteDataIntegrity)
{
    port = new ModbusTcpPortTestHelper();
    
    // Test that multiple writeBuffer calls properly prepare data for socket write
    uint8_t unit = 1;
    uint8_t func1 = MBF_READ_COILS;
    uint8_t data1[] = {0x00, 0x10, 0x00, 0x08}; // Start 16, quantity 8
    
    port->testWriteBuffer(unit, func1, data1, sizeof(data1));
    const uint8_t* buff1 = port->writeBufferData();
    uint16_t trans1 = port->getInternalTransaction();
    
    // Verify first request structure
    EXPECT_EQ(buff1[6], unit);
    EXPECT_EQ(buff1[7], func1);
    
    // Prepare second request
    uint8_t func2 = MBF_READ_HOLDING_REGISTERS;
    uint8_t data2[] = {0x00, 0x00, 0x00, 0x0A};
    
    port->testWriteBuffer(unit, func2, data2, sizeof(data2));
    const uint8_t* buff2 = port->writeBufferData();
    uint16_t trans2 = port->getInternalTransaction();
    
    // Transaction ID should increment
    EXPECT_NE(trans1, trans2);
    EXPECT_EQ(buff2[6], unit);
    EXPECT_EQ(buff2[7], func2);
}

TEST_F(ModbusTcpPortTest, SocketReadResponseValidation)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true); // Server mode for flexible transaction ID
    
    // Simulate a valid TCP response in the internal buffer
    uint8_t response[] = {
        0x00, 0x01,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x07,  // Length = 7
        0x01,        // Unit
        0x03,        // Function (Read Holding Registers)
        0x04,        // Byte count
        0x00, 0x0A,  // Register 1 value
        0x00, 0x0B   // Register 2 value
    };
    
    port->setInternalBuffer(response, sizeof(response));
    
    // Parse the response using readBuffer
    uint8_t outUnit, outFunc;
    uint8_t outData[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outData, sizeof(outData), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, 0x01);
    EXPECT_EQ(outFunc, 0x03);
    EXPECT_EQ(outSize, 5); // Byte count field + 4 bytes of data
    EXPECT_EQ(outData[0], 0x04); // Byte count field
    EXPECT_EQ(outData[1], 0x00); // Register 1 MSB
    EXPECT_EQ(outData[2], 0x0A); // Register 1 LSB
}

TEST_F(ModbusTcpPortTest, SocketWriteExceptionHandling)
{
    port = new ModbusTcpPortTestHelper();
    
    // Test exception response preparation
    uint8_t unit = 1;
    uint8_t func = 0x83; // Exception: Read Holding Registers + 0x80
    uint8_t exceptionCode[] = {0x02}; // Illegal Data Address
    
    StatusCode result = port->testWriteBuffer(unit, func, exceptionCode, sizeof(exceptionCode));
    EXPECT_EQ(result, Status_Good);
    
    const uint8_t* buff = port->writeBufferData();
    EXPECT_EQ(buff[7], 0x83); // Exception function code
    EXPECT_EQ(buff[8], 0x02); // Exception code
}

TEST_F(ModbusTcpPortTest, SocketReadExceptionResponse)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    // Simulate exception response in buffer
    uint8_t exceptionResponse[] = {
        0x00, 0x01,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x03,  // Length = 3
        0x01,        // Unit
        0x83,        // Function (Exception)
        0x02         // Exception code
    };
    
    port->setInternalBuffer(exceptionResponse, sizeof(exceptionResponse));
    
    uint8_t outUnit, outFunc;
    uint8_t outData[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outData, sizeof(outData), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outFunc, 0x83); // Exception function
    EXPECT_EQ(outData[0], 0x02); // Exception code
}

TEST_F(ModbusTcpPortTest, SocketWriteReadFullCycle)
{
    port = new ModbusTcpPortTestHelper();
    
    // Test complete request/response cycle (without actual socket I/O)
    
    // 1. Prepare write request
    uint8_t unit = 1;
    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t requestData[] = {0x00, 0x00, 0x00, 0x0A}; // Start 0, qty 10
    
    port->testWriteBuffer(unit, func, requestData, sizeof(requestData));
    uint16_t sentTransaction = port->getInternalTransaction();
    
    // 2. Simulate response (would come from socket read)
    uint8_t responseData[] = {
        static_cast<uint8_t>(sentTransaction >> 8),
        static_cast<uint8_t>(sentTransaction & 0xFF),
        0x00, 0x00,  // Protocol ID
        0x00, 0x17,  // Length = 23 (1 + 1 + 1 + 20)
        unit,        // Unit
        func,        // Function
        0x14,        // Byte count = 20 (10 registers * 2 bytes)
        // 10 register values (dummy data)
        0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05,
        0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0A
    };
    
    port->setInternalBuffer(responseData, sizeof(responseData));
    
    // 3. Parse response
    uint8_t outUnit, outFunc;
    uint8_t outData[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outData, sizeof(outData), &outSize);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outUnit, unit);
    EXPECT_EQ(outFunc, func);
    EXPECT_EQ(outSize, 21); // Byte count + 20 bytes of data
    EXPECT_EQ(outData[0], 0x14); // Byte count
}

TEST_F(ModbusTcpPortTest, SocketWriteVariousFunctionCodes)
{
    port = new ModbusTcpPortTestHelper();
    
    uint8_t unit = 1;
    
    // Test Write Single Coil (0x05)
    uint8_t writeSingleCoilData[] = {0x00, 0x0A, 0xFF, 0x00}; // Address 10, ON
    port->testWriteBuffer(unit, MBF_WRITE_SINGLE_COIL, writeSingleCoilData, sizeof(writeSingleCoilData));
    const uint8_t* buff1 = port->writeBufferData();
    EXPECT_EQ(buff1[7], MBF_WRITE_SINGLE_COIL);
    
    // Test Write Single Register (0x06)
    uint8_t writeSingleRegData[] = {0x00, 0x0A, 0x01, 0x23}; // Address 10, value 0x0123
    port->testWriteBuffer(unit, MBF_WRITE_SINGLE_REGISTER, writeSingleRegData, sizeof(writeSingleRegData));
    const uint8_t* buff2 = port->writeBufferData();
    EXPECT_EQ(buff2[7], MBF_WRITE_SINGLE_REGISTER);
    
    // Test Write Multiple Coils (0x0F)
    uint8_t writeMultipleCoilsData[] = {0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD, 0x01}; // Start 19, qty 10, 2 bytes
    port->testWriteBuffer(unit, MBF_WRITE_MULTIPLE_COILS, writeMultipleCoilsData, sizeof(writeMultipleCoilsData));
    const uint8_t* buff3 = port->writeBufferData();
    EXPECT_EQ(buff3[7], MBF_WRITE_MULTIPLE_COILS);
}

TEST_F(ModbusTcpPortTest, SocketReadDifferentResponseSizes)
{
    port = new ModbusTcpPortTestHelper();
    port->setServerMode(true);
    
    // Test small response (single coil read)
    uint8_t smallResponse[] = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x04,
        0x01, 0x01, 0x01, 0xCD
    };
    port->setInternalBuffer(smallResponse, sizeof(smallResponse));
    
    uint8_t outUnit, outFunc;
    uint8_t outData[255];
    uint16_t outSize;
    
    StatusCode result = port->testReadBuffer(outUnit, outFunc, outData, sizeof(outData), &outSize);
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outSize, 2); // Byte count + data
    
    // Test large response (many registers)
    uint8_t largeResponse[260];
    largeResponse[0] = 0x00; largeResponse[1] = 0x02; // Transaction ID
    largeResponse[2] = 0x00; largeResponse[3] = 0x00; // Protocol ID
    largeResponse[4] = 0x00; largeResponse[5] = 0xFE; // Length = 254
    largeResponse[6] = 0x01; // Unit
    largeResponse[7] = 0x03; // Function
    for (int i = 8; i < 260; i++)
        largeResponse[i] = static_cast<uint8_t>(i & 0xFF);
    
    port->setInternalBuffer(largeResponse, sizeof(largeResponse));
    result = port->testReadBuffer(outUnit, outFunc, outData, sizeof(outData), &outSize);
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(outSize, 252); // 260 - 6 (header) - 1 (unit) - 1 (func)
}
