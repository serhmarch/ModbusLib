#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusClientPort.h>
#include <ModbusClient.h>
#include <ModbusGlobal.h>

#include "MockModbusPort.h"

using namespace testing;
using namespace Modbus;

// ============================================================================
// Test Fixture for ModbusClientPort
// ============================================================================

class ModbusClientPortTest : public ::testing::Test
{
protected:
    struct SignalCounter
{
    uint32_t openCount     {0};
    uint32_t closeCount    {0};
    uint32_t txCount       {0};
    uint32_t rxCount       {0};
    uint32_t errorCount    {0};
    uint32_t completeCount {0};

    void onOpened(const Modbus::Char *)
    {
        openCount++;
    }

    void onClosed(const Modbus::Char *)
    {
        closeCount++;
    };

    void onTx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        txCount++;
    };

    void onRx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        rxCount++;
    };

    void onError(const Modbus::Char *, Modbus::StatusCode, const Modbus::Char *)
    {
        errorCount++;
    };

    void onCompleted(const Modbus::Char *, Modbus::StatusCode)
    {
        completeCount++;
    };
};


protected:
    MockModbusPort *mockPort;
    ModbusClientPort *clientPort;
    SignalCounter signalCounter;

    MockModbusPort *mockPortNonBlock {nullptr};
    ModbusClientPort *clientPortNonBlock {nullptr};
    SignalCounter signalCounterNonBlock;

    void SetUp() override
    {
        mockPort = new MockModbusPort(true);
        // Expect client to switch port into client mode
        EXPECT_CALL(*mockPort, setServerMode(false)).Times(AtLeast(0));
        //mockPort->setTimeout(1); // Set minimal timeout for tests
        clientPort = new ModbusClientPort(mockPort);
    }

    void TearDown() override
    {
        delete clientPort;
        delete clientPortNonBlock;
        clientPortNonBlock = nullptr;
        // mockPort and mockPortNonBlock are deleted by clientPort and clientPortNonBlock respectively,
        // so we don't delete them here to avoid double deletion.
    }

    // Helper to setup successful write/read cycle
    void setupSuccessfulTransaction(uint8_t unit, uint8_t func, 
                                    const uint8_t *requestData, uint16_t requestSize,
                                    const uint8_t *responseData, uint16_t responseSize)
    {
        // Reset signal counters
        signalCounter = SignalCounter();
        clientPort->connect(&ModbusClientPort::signalOpened   , &signalCounter, &SignalCounter::onOpened   );
        clientPort->connect(&ModbusClientPort::signalClosed   , &signalCounter, &SignalCounter::onClosed   );
        clientPort->connect(&ModbusClientPort::signalTx       , &signalCounter, &SignalCounter::onTx       );
        clientPort->connect(&ModbusClientPort::signalRx       , &signalCounter, &SignalCounter::onRx       );
        clientPort->connect(&ModbusClientPort::signalError    , &signalCounter, &SignalCounter::onError    );
        clientPort->connect(&ModbusClientPort::signalCompleted, &signalCounter, &SignalCounter::onCompleted);

        EXPECT_CALL(*mockPort, isOpen())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(*mockPort, writeBuffer(unit, func, _, requestSize))
            .WillOnce(Return(Status_Good));
        
        // Mock buffer size/data methods to eliminate warnings
        EXPECT_CALL(*mockPort, writeBufferSize())
            .WillRepeatedly(Return(requestSize));
        
        EXPECT_CALL(*mockPort, writeBufferData())
            .WillRepeatedly(Return(requestData));

        EXPECT_CALL(*mockPort, write())
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPort, read())
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(unit),
                SetArgReferee<1>(func),
                SetArrayArgument<2>(responseData, responseData + responseSize),
                SetArgPointee<4>(responseSize),
                Return(Status_Good)));
        
        EXPECT_CALL(*mockPort, readBufferSize())
            .WillRepeatedly(Return(responseSize));
        
        EXPECT_CALL(*mockPort, readBufferData())
            .WillRepeatedly(Return(responseData));
    }

    // Helper to setup successful write/read cycle
    void setupSuccessfulNonBlockTransaction(uint8_t unit, uint8_t func, 
                                            const uint8_t *requestData, uint16_t requestSize,
                                            const uint8_t *responseData, uint16_t responseSize)
    {
        mockPortNonBlock = new MockModbusPort(false);

        // Expect client to switch port into client mode
        EXPECT_CALL(*mockPortNonBlock, setServerMode(false)).Times(AtLeast(0));
        //mockPortNonBlock->setTimeout(1); // Set minimal timeout for tests
        clientPortNonBlock = new ModbusClientPort(mockPortNonBlock);

        // Reset signal counters
        signalCounterNonBlock = SignalCounter();
        clientPortNonBlock->connect(&ModbusClientPort::signalOpened   , &signalCounterNonBlock, &SignalCounter::onOpened   );
        clientPortNonBlock->connect(&ModbusClientPort::signalClosed   , &signalCounterNonBlock, &SignalCounter::onClosed   );
        clientPortNonBlock->connect(&ModbusClientPort::signalTx       , &signalCounterNonBlock, &SignalCounter::onTx       );
        clientPortNonBlock->connect(&ModbusClientPort::signalRx       , &signalCounterNonBlock, &SignalCounter::onRx       );
        clientPortNonBlock->connect(&ModbusClientPort::signalError    , &signalCounterNonBlock, &SignalCounter::onError    );
        clientPortNonBlock->connect(&ModbusClientPort::signalCompleted, &signalCounterNonBlock, &SignalCounter::onCompleted);

        EXPECT_CALL(*mockPortNonBlock, isOpen())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(*mockPortNonBlock, writeBuffer(unit, func, _, requestSize))
            .WillOnce(Return(Status_Good));
        
        // Mock buffer size/data methods to eliminate warnings
        EXPECT_CALL(*mockPortNonBlock, writeBufferSize())
            .WillRepeatedly(Return(requestSize));
        
        EXPECT_CALL(*mockPortNonBlock, writeBufferData())
            .WillRepeatedly(Return(requestData));

        EXPECT_CALL(*mockPortNonBlock, write())
            .WillOnce(Return(Status_Processing))
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPortNonBlock, read())
            .WillOnce(Return(Status_Processing))
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPortNonBlock, readBuffer(_, _, _, _, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(unit),
                SetArgReferee<1>(func),
                SetArrayArgument<2>(responseData, responseData + responseSize),
                SetArgPointee<4>(responseSize),
                Return(Status_Good)));
        
        EXPECT_CALL(*mockPortNonBlock, readBufferSize())
            .WillRepeatedly(Return(responseSize));
        
        EXPECT_CALL(*mockPortNonBlock, readBufferData())
            .WillRepeatedly(Return(responseData));
    }
};


// ============================================================================
// Basic Initialization and Configuration Tests
// ============================================================================

TEST_F(ModbusClientPortTest, Constructor)
{
    EXPECT_NE(clientPort, nullptr);
    EXPECT_EQ(clientPort->port(), mockPort);
}

TEST_F(ModbusClientPortTest, TypeReturnsPortType)
{
    EXPECT_CALL(*mockPort, type())
        .WillOnce(Return(ProtocolType::TCP));
    
    EXPECT_EQ(clientPort->type(), ProtocolType::TCP);
}

TEST_F(ModbusClientPortTest, PortGetter)
{
    EXPECT_EQ(clientPort->port(), mockPort);
}

TEST_F(ModbusClientPortTest, SetPort)
{
    NiceMock<MockModbusPort> *newPort = new NiceMock<MockModbusPort>(true);
    
    EXPECT_CALL(*mockPort, close())
                .WillOnce(Return(Status_Good));

    clientPort->setPort(newPort);
    
    EXPECT_EQ(clientPort->port(), newPort);
}

TEST_F(ModbusClientPortTest, IsOpenDelegatesToPort)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    
    EXPECT_TRUE(clientPort->isOpen());
    EXPECT_FALSE(clientPort->isOpen());
}

TEST_F(ModbusClientPortTest, CloseDelegatesToPort)
{
    EXPECT_CALL(*mockPort, close())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = clientPort->close();
    
    EXPECT_EQ(result, Status_Good);
}

TEST_F(ModbusClientPortTest, TriesDefaultValue)
{
    EXPECT_EQ(clientPort->tries(), 1u);
}

TEST_F(ModbusClientPortTest, SetTries)
{
    clientPort->setTries(3);
    EXPECT_EQ(clientPort->tries(), 3u);
}

TEST_F(ModbusClientPortTest, RepeatCountBackwardCompatibility)
{
    clientPort->setRepeatCount(5);
    EXPECT_EQ(clientPort->repeatCount(), 5u);
    EXPECT_EQ(clientPort->tries(), 5u);
}

TEST_F(ModbusClientPortTest, BroadcastEnabledByDefault)
{
    EXPECT_TRUE(clientPort->isBroadcastEnabled());
}

TEST_F(ModbusClientPortTest, SetBroadcastEnabled)
{
    clientPort->setBroadcastEnabled(false);
    EXPECT_FALSE(clientPort->isBroadcastEnabled());
    
    clientPort->setBroadcastEnabled(true);
    EXPECT_TRUE(clientPort->isBroadcastEnabled());
}

// ============================================================================
// Read Coils Tests (Function Code 0x01)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadCoilsSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    uint8_t requestData[4];
    requestData[0] = 0x00; // offset high
    requestData[1] = 0x00; // offset low
    requestData[2] = 0x00; // count high
    requestData[3] = 0x08; // count low
    
    uint8_t responseData[2];
    responseData[0] = 0x01; // byte count
    responseData[1] = 0xAA; // coil values
    
    setupSuccessfulTransaction(unit, MBF_READ_COILS, requestData, 4, responseData, 2);
    
    uint8_t values[1];

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value 

    StatusCode result = clientPort->readCoils(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0xAA);

    // Non-blocking version
    values[0] = 0; // Clear values buffer before non-blocking test  
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_COILS, requestData, 4, responseData, 2);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value 

    result = clientPortNonBlock->readCoils(unit, offset, count, values);    
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing 
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet 
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet 

    result = clientPortNonBlock->readCoils(unit, offset, count, values);    
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readCoils(unit, offset, count, values);    
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0xAA); // Verify that values buffer is correctly filled after non-blocking operation completes
}

TEST_F(ModbusClientPortTest, ReadCoilsWithClient)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x08};
    uint8_t responseData[2] = {0x01, 0x55};
    
    setupSuccessfulTransaction(unit, MBF_READ_COILS, requestData, 4, responseData, 2);
    
    ModbusClient client(unit, clientPort);
    uint8_t values[1];
    StatusCode result = clientPort->readCoils(&client, unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(values[0], 0x55);
}

TEST_F(ModbusClientPortTest, ReadCoilsAsBoolArray)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x08};
    uint8_t responseData[2] = {0x01, 0b10101010};
    
    setupSuccessfulTransaction(unit, MBF_READ_COILS, requestData, 4, responseData, 2);
    
    bool values[8];
    StatusCode result = clientPort->readCoilsAsBoolArray(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_FALSE(values[0]);
    EXPECT_TRUE (values[1]);
    EXPECT_FALSE(values[2]);
    EXPECT_TRUE (values[3]);
}

// ============================================================================
// Read Discrete Inputs Tests (Function Code 0x02)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadDiscreteInputsSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const uint16_t count = 8;
    
    uint8_t requestData[4] = {0x00, 0x0A, 0x00, 0x08};
    uint8_t responseData[2] = {0x01, 0xF0};
    
    setupSuccessfulTransaction(unit, MBF_READ_DISCRETE_INPUTS, requestData, 4, responseData, 2);
    
    uint8_t values[1];

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->readDiscreteInputs(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0xF0);

    // Non-blocking version
    values[0] = 0; // Clear values buffer before non-blocking test
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_DISCRETE_INPUTS, requestData, 4, responseData, 2);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->readDiscreteInputs(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readDiscreteInputs(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readDiscreteInputs(unit, offset, count, values);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0xF0); // Verify that values buffer is correctly filled after non-blocking operation completes
}

TEST_F(ModbusClientPortTest, ReadDiscreteInputsAsBoolArray)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x08};
    uint8_t responseData[2] = {0x01, 0xFF};
    
    setupSuccessfulTransaction(unit, MBF_READ_DISCRETE_INPUTS, requestData, 4, responseData, 2);
    
    bool values[8];
    StatusCode result = clientPort->readDiscreteInputsAsBoolArray(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    for (int i = 0; i < 8; i++)
        EXPECT_TRUE(values[i]);
}

// ============================================================================
// Read Holding Registers Tests (Function Code 0x03)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadHoldingRegistersSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 2;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02};
    uint8_t responseData[5] = {0x04, 0x00, 0x0A, 0x00, 0x14}; // byte count + 2 registers
    
    setupSuccessfulTransaction(unit, MBF_READ_HOLDING_REGISTERS, requestData, 4, responseData, 5);
    
    uint16_t values[2];

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->readHoldingRegisters(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0x000A);
    EXPECT_EQ(values[1], 0x0014);

    // Non-blocking version
    values[0] = 0; // Clear values buffer before non-blocking test
    values[1] = 0;
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_HOLDING_REGISTERS, requestData, 4, responseData, 5);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0x000A); // Verify that values buffer is correctly filled after non-blocking operation completes
    EXPECT_EQ(values[1], 0x0014);
}

TEST_F(ModbusClientPortTest, ReadHoldingRegistersLargeCount)
{
    const uint8_t unit = 1;
    const uint16_t offset = 100;
    const uint16_t count = 10;
    
    uint8_t requestData[4];
    requestData[0] = 0x00;
    requestData[1] = 0x64;
    requestData[2] = 0x00;
    requestData[3] = 0x0A;
    
    uint8_t responseData[21];
    responseData[0] = 0x14; // byte count = 20
    for (int i = 0; i < 20; i++)
        responseData[i + 1] = static_cast<uint8_t>(i);
    
    setupSuccessfulTransaction(unit, MBF_READ_HOLDING_REGISTERS, requestData, 4, responseData, 21);
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
}

// ============================================================================
// Read Input Registers Tests (Function Code 0x04)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadInputRegistersSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 5;
    const uint16_t count = 3;
    
    uint8_t requestData[4] = {0x00, 0x05, 0x00, 0x03};
    uint8_t responseData[7] = {0x06, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
    
    setupSuccessfulTransaction(unit, MBF_READ_INPUT_REGISTERS, requestData, 4, responseData, 7);
    
    uint16_t values[3];

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->readInputRegisters(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0x1234);
    EXPECT_EQ(values[1], 0x5678);
    EXPECT_EQ(values[2], 0x9ABC);

    // Non-blocking version
    values[0] = 0; // Clear values buffer before non-blocking test
    values[1] = 0;
    values[2] = 0;
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_INPUT_REGISTERS, requestData, 4, responseData, 7);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->readInputRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readInputRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readInputRegisters(unit, offset, count, values);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(values[0], 0x1234); // Verify that values buffer is correctly filled after non-blocking operation completes
    EXPECT_EQ(values[1], 0x5678);
    EXPECT_EQ(values[2], 0x9ABC);
}

// ============================================================================
// Write Single Coil Tests (Function Code 0x05)
// ============================================================================

TEST_F(ModbusClientPortTest, WriteSingleCoilOn)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const bool value = true;
    
    uint8_t requestData[4] = {0x00, 0x0A, 0xFF, 0x00};
    uint8_t responseData[4] = {0x00, 0x0A, 0xFF, 0x00};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_SINGLE_COIL, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->writeSingleCoil(unit, offset, value);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_WRITE_SINGLE_COIL, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

TEST_F(ModbusClientPortTest, WriteSingleCoilOff)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const bool value = false;
    
    uint8_t requestData[4] = {0x00, 0x0A, 0x00, 0x00};
    uint8_t responseData[4] = {0x00, 0x0A, 0x00, 0x00};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_SINGLE_COIL, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->writeSingleCoil(unit, offset, value);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_WRITE_SINGLE_COIL, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

// ============================================================================
// Write Single Register Tests (Function Code 0x06)
// ============================================================================

TEST_F(ModbusClientPortTest, WriteSingleRegisterSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 20;
    const uint16_t value = 0x1234;
    
    uint8_t requestData[4] = {0x00, 0x14, 0x12, 0x34};
    uint8_t responseData[4] = {0x00, 0x14, 0x12, 0x34};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_SINGLE_REGISTER, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->writeSingleRegister(unit, offset, value);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_WRITE_SINGLE_REGISTER, requestData, 4, responseData, 4);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->writeSingleRegister(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleRegister(unit, offset, value);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeSingleRegister(unit, offset, value);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

// ============================================================================
// Read Exception Status Tests (Function Code 0x07)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadExceptionStatusSuccess)
{
    const uint8_t unit = 1;
    
    uint8_t responseData[1] = {0x42}; // exception status byte
    
    setupSuccessfulTransaction(unit, MBF_READ_EXCEPTION_STATUS, nullptr, 0, responseData, 1);
    
    uint8_t status;
    StatusCode result = clientPort->readExceptionStatus(unit, &status);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(status, 0x42);
}

// ============================================================================
// Write Multiple Coils Tests (Function Code 0x0F)
// ============================================================================

TEST_F(ModbusClientPortTest, WriteMultipleCoilsSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const uint16_t count = 10;
    
    uint8_t coilValues[2] = {0xFF, 0x03}; // 10 coils
    
    uint8_t requestData[7] = {0x00, 0x0A, 0x00, 0x0A, 0x02, 0xFF, 0x03};
    uint8_t responseData[4] = {0x00, 0x0A, 0x00, 0x0A};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_MULTIPLE_COILS, requestData, 7, responseData, 4);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->writeMultipleCoils(unit, offset, count, coilValues);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_WRITE_MULTIPLE_COILS, requestData, 7, responseData, 4);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->writeMultipleCoils(unit, offset, count, coilValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeMultipleCoils(unit, offset, count, coilValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeMultipleCoils(unit, offset, count, coilValues);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

TEST_F(ModbusClientPortTest, WriteMultipleCoilsAsBoolArray)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    bool coilValues[8] = {true, false, true, false, true, false, true, false};
    
    uint8_t requestData[6] = {0x00, 0x00, 0x00, 0x08, 0x01, 0x55};
    uint8_t responseData[4] = {0x00, 0x00, 0x00, 0x08};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_MULTIPLE_COILS, requestData, 6, responseData, 4);
    
    StatusCode result = clientPort->writeMultipleCoilsAsBoolArray(unit, offset, count, coilValues);
    
    EXPECT_EQ(result, Status_Good);
}

// ============================================================================
// Write Multiple Registers Tests (Function Code 0x10)
// ============================================================================

TEST_F(ModbusClientPortTest, WriteMultipleRegistersSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 100;
    const uint16_t count = 2;
    
    uint16_t regValues[2] = {0x1234, 0x5678};
    
    uint8_t requestData[9] = {0x00, 0x64, 0x00, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78};
    uint8_t responseData[4] = {0x00, 0x64, 0x00, 0x02};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_MULTIPLE_REGISTERS, requestData, 9, responseData, 4);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->writeMultipleRegisters(unit, offset, count, regValues);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_WRITE_MULTIPLE_REGISTERS, requestData, 9, responseData, 4);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->writeMultipleRegisters(unit, offset, count, regValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeMultipleRegisters(unit, offset, count, regValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->writeMultipleRegisters(unit, offset, count, regValues);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

// ============================================================================
// Mask Write Register Tests (Function Code 0x16)
// ============================================================================

TEST_F(ModbusClientPortTest, MaskWriteRegisterSuccess)
{
    const uint8_t unit = 1;
    const uint16_t offset = 50;
    const uint16_t andMask = 0xFF00;
    const uint16_t orMask = 0x0012;
    
    uint8_t requestData[6] = {0x00, 0x32, 0xFF, 0x00, 0x00, 0x12};
    uint8_t responseData[6] = {0x00, 0x32, 0xFF, 0x00, 0x00, 0x12};
    
    setupSuccessfulTransaction(unit, MBF_MASK_WRITE_REGISTER, requestData, 6, responseData, 6);

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->maskWriteRegister(unit, offset, andMask, orMask);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    // Non-blocking version
    setupSuccessfulNonBlockTransaction(unit, MBF_MASK_WRITE_REGISTER, requestData, 6, responseData, 6);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->maskWriteRegister(unit, offset, andMask, orMask);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->maskWriteRegister(unit, offset, andMask, orMask);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->maskWriteRegister(unit, offset, andMask, orMask);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete
}

// ============================================================================
// Read Write Multiple Registers Tests (Function Code 0x17)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadWriteMultipleRegistersSuccess)
{
    const uint8_t unit = 1;
    const uint16_t readOffset = 0;
    const uint16_t readCount = 2;
    const uint16_t writeOffset = 100;
    const uint16_t writeCount = 2;
    
    uint16_t writeValues[2] = {0xABCD, 0xEF01};
    uint16_t readValues[2];
    
    uint8_t requestData[13] = {0x00, 0x00, 0x00, 0x02, 0x00, 0x64, 0x00, 0x02, 0x04, 0xAB, 0xCD, 0xEF, 0x01};
    uint8_t responseData[5] = {0x04, 0x12, 0x34, 0x56, 0x78};
    
    setupSuccessfulTransaction(unit, MBF_READ_WRITE_MULTIPLE_REGISTERS, requestData, 13, responseData, 5);
    
    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(readValues[0], 0x1234);
    EXPECT_EQ(readValues[1], 0x5678);

    // Non-blocking version
    readValues[0] = 0; // Clear values buffer before non-blocking test
    readValues[1] = 0;
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_WRITE_MULTIPLE_REGISTERS, requestData, 13, responseData, 5);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readWriteMultipleRegisters(unit, readOffset, readCount, readValues, writeOffset, writeCount, writeValues);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(readValues[0], 0x1234); // Verify that values buffer is correctly filled after non-blocking operation completes
    EXPECT_EQ(readValues[1], 0x5678);
}

// ============================================================================
// Read FIFO Queue Tests (Function Code 0x18)
// ============================================================================

TEST_F(ModbusClientPortTest, ReadFIFOQueueSuccess)
{
    const uint8_t unit = 1;
    const uint16_t fifoAddr = 10;
    
    uint8_t requestData[2] = {0x00, 0x0A};
    uint8_t responseData[10] = {0x00, 0x08, 0x00, 0x03, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // byte count (2), fifo count (2), 3 registers (6)
    
    setupSuccessfulTransaction(unit, MBF_READ_FIFO_QUEUE, requestData, 2, responseData, 10);
    
    uint16_t count;
    uint16_t values[3];

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    StatusCode result = clientPort->readFIFOQueue(unit, fifoAddr, &count, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(count, 3);

    // Non-blocking version
    count = 0; // Clear count before non-blocking test
    setupSuccessfulNonBlockTransaction(unit, MBF_READ_FIFO_QUEUE, requestData, 2, responseData, 10);

    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Check init value

    result = clientPortNonBlock->readFIFOQueue(unit, fifoAddr, &count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 0); // Tx signal should not be emitted yet because write() is still processing
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is not called yet
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readFIFOQueue(unit, fifoAddr, &count, values);
    EXPECT_EQ(result, Status_Processing); // First call write() returns Good, but read() returns Processing
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 0); // Rx signal should not be emitted yet because read() is still processing
    EXPECT_EQ(signalCounterNonBlock.completeCount, 0); // Complete signal should not be emitted yet because operation is not complete yet

    result = clientPortNonBlock->readFIFOQueue(unit, fifoAddr, &count, values);
    EXPECT_EQ(result, Status_Good); // read() call returns Good, operation should be complete and returns Good as well
    EXPECT_EQ(signalCounterNonBlock.txCount      , 1); // Tx signal should be emitted because write() returned Good
    EXPECT_EQ(signalCounterNonBlock.rxCount      , 1); // Rx signal should be emitted because read() returned Good
    EXPECT_EQ(signalCounterNonBlock.completeCount, 1); // Complete signal should be emitted because operation is complete

    EXPECT_EQ(count, 3); // Verify that count is correctly filled after non-blocking operation completes
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(ModbusClientPortTest, PortNotOpen)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(false));
    
    EXPECT_CALL(*mockPort, open())
        .WillRepeatedly(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillRepeatedly(Return(Status_Good));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(1, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadPortClosed);
}

TEST_F(ModbusClientPortTest, WriteBufferError)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_BadWriteBufferOverflow));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadWriteBufferOverflow);
}

TEST_F(ModbusClientPortTest, WriteError)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_BadTcpWrite));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadTcpWrite);
}

TEST_F(ModbusClientPortTest, ReadError)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(4));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_BadSerialReadTimeout));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadSerialReadTimeout);
}

TEST_F(ModbusClientPortTest, ReadBufferError)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(4));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(Return(Status_BadCrc));
    
    EXPECT_CALL(*mockPort, readBufferSize())
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*mockPort, readBufferData())
        .WillRepeatedly(Return(nullptr));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadCrc);
}

TEST_F(ModbusClientPortTest, ExceptionResponse)
{
    const uint8_t unit = 1;
    const uint8_t func = MBF_READ_HOLDING_REGISTERS;
    const uint8_t exceptionFunc = func | 0x80;
    const uint8_t exceptionCode = 0x02; // Illegal data address
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(4));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    uint8_t responseData[1] = {exceptionCode};
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(exceptionFunc),
            SetArrayArgument<2>(responseData, responseData + 1),
            SetArgPointee<4>(1),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, readBufferSize())
        .WillRepeatedly(Return(1));
    
    EXPECT_CALL(*mockPort, readBufferData())
        .WillRepeatedly(Return(responseData));
    
    uint16_t values[10];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 10, values);
    
    EXPECT_EQ(result, Status_BadIllegalDataAddress);
}

// ============================================================================
// Retry Mechanism Tests
// ============================================================================

TEST_F(ModbusClientPortTest, RetryOnFailure)
{
    const uint8_t unit = 1;
    clientPort->setTries(3);
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Note: No need to fill write buffer on every try, just once at first try
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(1)
        .WillRepeatedly(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(4));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .Times(3)
        .WillRepeatedly(Return(Status_Good));
    
    // First two reads fail, third succeeds
    EXPECT_CALL(*mockPort, read())
        .Times(3)
        .WillOnce(Return(Status_BadSerialReadTimeout))
        .WillOnce(Return(Status_BadSerialReadTimeout))
        .WillOnce(Return(Status_Good));
    
    uint8_t responseData[5] = {0x04, 0x00, 0x0A, 0x00, 0x14};
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(responseData, responseData + 5),
            SetArgPointee<4>(5),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, readBufferSize())
        .WillRepeatedly(Return(5));
    
    EXPECT_CALL(*mockPort, readBufferData())
        .WillRepeatedly(Return(responseData));
    
    uint16_t values[2];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 2, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(clientPort->lastTries(), 3);
}

TEST_F(ModbusClientPortTest, AllRetriesFail)
{
    const uint8_t unit = 1;
    clientPort->setTries(2);
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Note: No need to fill write buffer on every try, just once at first try
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(1)
        .WillRepeatedly(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(4));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .Times(2)
        .WillRepeatedly(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, read())
        .Times(2)
        .WillRepeatedly(Return(Status_BadSerialReadTimeout));
    
    uint16_t values[2];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 2, values);
    
    EXPECT_EQ(result, Status_BadSerialReadTimeout);
    EXPECT_EQ(clientPort->lastTries(), 2);
}

// ============================================================================
// Status Tracking Tests
// ============================================================================

TEST_F(ModbusClientPortTest, LastStatusTracking)
{
    const uint8_t unit = 1;
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02};
    uint8_t responseData[5] = {0x04, 0x00, 0x0A, 0x00, 0x14};
    
    setupSuccessfulTransaction(unit, MBF_READ_HOLDING_REGISTERS, requestData, 4, responseData, 5);
    
    uint16_t values[2];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 2, values);
    
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(clientPort->lastStatus(), Status_Good);
}

TEST_F(ModbusClientPortTest, LastErrorStatusTracking)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_BadTcpDisconnect));
    
    uint16_t values[2];
    StatusCode result = clientPort->readHoldingRegisters(unit, 0, 2, values);
    
    EXPECT_EQ(result, Status_BadTcpDisconnect);
    EXPECT_EQ(clientPort->lastErrorStatus(), Status_BadTcpDisconnect);
}

TEST_F(ModbusClientPortTest, LastErrorTextAvailable)
{
    const uint8_t unit = 1;
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_BadSerialWriteTimeout));
    
    uint16_t values[2];
    clientPort->readHoldingRegisters(unit, 0, 2, values);
    
    const Char *errorText = clientPort->lastErrorText();
    EXPECT_NE(errorText, nullptr);
}

// ============================================================================
// Broadcast Mode Tests
// ============================================================================

TEST_F(ModbusClientPortTest, BroadcastModeUnit0)
{
    const uint8_t unit = 0; // Broadcast address
    const uint16_t offset = 100;
    const uint16_t value = 0x1234;
    
    clientPort->setBroadcastEnabled(true);
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_REGISTER, _, 4))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(nullptr));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    // In broadcast mode, no response is expected
    EXPECT_CALL(*mockPort, read())
        .Times(0);
    
    StatusCode result = clientPort->writeSingleRegister(unit, offset, value);
    
    EXPECT_EQ(result, Status_Good);
}

TEST_F(ModbusClientPortTest, BroadcastDisabled)
{
    const uint8_t unit = 0;
    const uint16_t offset = 100;
    const uint16_t value = 0x1234;
    
    clientPort->setBroadcastEnabled(false);
    
    uint8_t requestData[4] = {0x00, 0x64, 0x12, 0x34};
    uint8_t responseData[4] = {0x00, 0x64, 0x12, 0x34};
    
    setupSuccessfulTransaction(unit, MBF_WRITE_SINGLE_REGISTER, requestData, 4, responseData, 4);
    
    StatusCode result = clientPort->writeSingleRegister(unit, offset, value);
    
    EXPECT_EQ(result, Status_Good);
}

// ============================================================================
// Algorithm Test (Similar to ServerPort test)
// ============================================================================

TEST(ModbusClientPort, testAlgorithmBlocking)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>(true);
    
    ModbusClientPort clientPort(port);
    
    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;
    
    EXPECT_CALL(*port, isOpen())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    
    // Request preparation
    uint8_t requestData[4];
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    
    EXPECT_CALL(*port, writeBuffer(unit, func, _, 4))
        .Times(1)
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));
    
    // Response data: byte count + 16 registers (32 bytes)
    uint8_t responseData[33];
    responseData[0] = 32; // byte count
    for (int i = 0; i < 32; i++)
        responseData[i + 1] = static_cast<uint8_t>(i);
    
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(responseData, responseData + 33),
            SetArgPointee<4>(33),
            Return(Status_Good)));
    
    uint16_t values[count];
    StatusCode result = clientPort.readHoldingRegisters(unit, offset, count, values);
    
    EXPECT_EQ(result, Status_Good);
}

// ============================================================================
// Test signals emitted by ModbusClientPort (e.g., opened, closed etc.)
// ============================================================================

struct ModbusClientPortSignalTest
{
    uint32_t openCount     {0};
    uint32_t closeCount    {0};
    uint32_t txCount       {0};
    uint32_t rxCount       {0};
    uint32_t errorCount    {0};
    uint32_t completeCount {0};

    void onOpened(const Modbus::Char *)
    {
        openCount++;
    }

    void onClosed(const Modbus::Char *)
    {
        closeCount++;
    };

    void onTx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        txCount++;
    };

    void onRx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        rxCount++;
    };

    void onError(const Modbus::Char *, Modbus::StatusCode, const Modbus::Char *)
    {
        errorCount++;
    };

    void onCompleted(const Modbus::Char *, Modbus::StatusCode)
    {
        completeCount++;
    };
};

TEST(ModbusClientPort, testSignals)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>(true);
    
    ModbusClientPort clientPort(port);
    ModbusClientPortSignalTest signalHandler;

    clientPort.connect(&ModbusClientPort::signalOpened   , &signalHandler, &ModbusClientPortSignalTest::onOpened   );
    clientPort.connect(&ModbusClientPort::signalClosed   , &signalHandler, &ModbusClientPortSignalTest::onClosed   );
    clientPort.connect(&ModbusClientPort::signalTx       , &signalHandler, &ModbusClientPortSignalTest::onTx       );
    clientPort.connect(&ModbusClientPort::signalRx       , &signalHandler, &ModbusClientPortSignalTest::onRx       );
    clientPort.connect(&ModbusClientPort::signalError    , &signalHandler, &ModbusClientPortSignalTest::onError    );
    clientPort.connect(&ModbusClientPort::signalCompleted, &signalHandler, &ModbusClientPortSignalTest::onCompleted);

    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;
    
    // Request preparation
    uint8_t requestData[4];
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    
    // Response data: byte count + 16 registers (32 bytes)
    const auto szResponse = 33;
    uint8_t responseData[szResponse];
    responseData[0] = 32; // byte count
    for (int i = 0; i < 32; i++)
        responseData[i + 1] = static_cast<uint8_t>(i);
    

    EXPECT_CALL(*port, isOpen())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*port, writeBuffer(unit, func, _, 4))
        .WillOnce(Return(Status_Good)) // Step 1
        .WillOnce(Return(Status_BadWriteBufferOverflow )) // Step 2
        .WillOnce(Return(Status_Good)) // Step 3
        .WillOnce(Return(Status_Good)) // Step 4
        .WillOnce(Return(Status_Good)) // Step 5
        .WillOnce(Return(Status_Good)) // Step 6
        ;
    
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good)) // Step 1
        .WillOnce(Return(Status_Bad )) // Step 3
        .WillOnce(Return(Status_Good)) // Step 4
        .WillOnce(Return(Status_Good)) // Step 5
        .WillOnce(Return(Status_Good)) // Step 6
        ;
    
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good)) // Step 1
        .WillOnce(Return(Status_Bad )) // Step 4
        .WillOnce(Return(Status_Good)) // Step 5
        .WillOnce(Return(Status_Good)) // Step 6
        ;
    
    EXPECT_CALL(*port, readBufferSize()) // Each step
        .WillRepeatedly(Return(szResponse))
        ;
    
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillRepeatedly(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(responseData, responseData + szResponse),
            SetArgPointee<4>(szResponse),
            Return(Status_Good)));
    
    uint32_t expected_openCount     {0};
    uint32_t expected_closeCount    {0};
    uint32_t expected_txCount       {0};
    uint32_t expected_rxCount       {0};
    uint32_t expected_errorCount    {0};
    uint32_t expected_completeCount {0};

    uint16_t values[count];

    // Step 1: Successful transaction
    StatusCode result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    , ++expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));
    
    // Step 2: Fill buffer data error
    result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      ,   expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));
    
    // Step 3: Write port error
    result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      ,   expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));

    // Step 4: Read port error
    result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      ,   expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));

    // Step 5: Successful transaction
    result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));
    
    // Step 6: Read port success but closed
    EXPECT_CALL(*port, isOpen())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));    

    result = clientPort.readHoldingRegisters(unit, offset, count, values);
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   , ++expected_closeCount   );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));

}

// ============================================================================
// Test multiple clients
// ============================================================================

TEST(ModbusClientPort, testMultipleClients)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *mockPort = new NiceMock<MockModbusPort>(true);

    uint8_t func = MBF_READ_HOLDING_REGISTERS;
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x02};
    uint8_t responseData[5] = {0x04, 0x00, 0x0A, 0x00, 0x14};
    uint16_t requestSize = sizeof(requestData);
    uint16_t responseSize = sizeof(responseData);

    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockPort, writeBuffer(_, func, _, requestSize))
        .WillRepeatedly(Return(Status_Good));
    
    // Mock buffer size/data methods to eliminate warnings
    EXPECT_CALL(*mockPort, writeBufferSize())
        .WillRepeatedly(Return(requestSize));
    
    EXPECT_CALL(*mockPort, writeBufferData())
        .WillRepeatedly(Return(requestData));

    EXPECT_CALL(*mockPort, write())
        .WillRepeatedly(Return(Status_Good));

    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Processing))
        .WillOnce(Return(Status_Good))
        .WillOnce(Return(Status_Processing))
        .WillOnce(Return(Status_Good))
        .WillOnce(Return(Status_Processing))
        .WillOnce(Return(Status_Good))
        ;

    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillRepeatedly(DoAll(
                SetArgReferee<1>(func),
                SetArrayArgument<2>(responseData, responseData + responseSize),
                SetArgPointee<4>(responseSize),
                Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, readBufferSize())
        .WillRepeatedly(Return(responseSize));
    
    EXPECT_CALL(*mockPort, readBufferData())
        .WillRepeatedly(Return(responseData));

    ModbusClientPort clientPort(mockPort);
    auto signalCounter = ModbusClientPortSignalTest();
    clientPort.connect(&ModbusClientPort::signalTx       , &signalCounter, &ModbusClientPortSignalTest::onTx       );
    clientPort.connect(&ModbusClientPort::signalRx       , &signalCounter, &ModbusClientPortSignalTest::onRx       );
    clientPort.connect(&ModbusClientPort::signalCompleted, &signalCounter, &ModbusClientPortSignalTest::onCompleted);

    ModbusClient client1(1, &clientPort);
    ModbusClient client2(2, &clientPort);
    ModbusClient client3(3, &clientPort);

    uint16_t readValues[2];
    StatusCode status1, status2, status3;

    EXPECT_EQ(signalCounter.txCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.rxCount      , 0); // Check init value
    EXPECT_EQ(signalCounter.completeCount, 0); // Check init value

    // -----------------------------------------------------------------------------
    // Step 1. All client begin their operation but 1st client holds the client port
    status1 = client1.readHoldingRegisters(0, 2, readValues);
    status2 = client2.readHoldingRegisters(0, 2, readValues);
    status3 = client3.readHoldingRegisters(0, 2, readValues);

    EXPECT_EQ(status1, Status_Processing);
    EXPECT_EQ(status2, Status_Processing);
    EXPECT_EQ(status3, Status_Processing);

    EXPECT_EQ(signalCounter.txCount      , 1); // Tx signal should be emitted
    EXPECT_EQ(signalCounter.rxCount      , 0); // Rx signal should not be emitted because read() is returns Processing for 1st client
    EXPECT_EQ(signalCounter.completeCount, 0); // Complete signal should not be emitted yet because all operations are not complete yet

    EXPECT_EQ(clientPort.currentClient(), &client1); // First client should be current client

    // -----------------------------------------------------------------------------
    // Step 2. 1st client's read() returns Good, so 2nd client becomes current client
    // and its read() is called, but 3rd client's read() is not called yet because
    // 2nd client's read() is still processing
    status1 = client1.readHoldingRegisters(0, 2, readValues);
    status2 = client2.readHoldingRegisters(0, 2, readValues);
    status3 = client3.readHoldingRegisters(0, 2, readValues);

    EXPECT_EQ(status1, Status_Good      );
    EXPECT_EQ(status2, Status_Processing);
    EXPECT_EQ(status3, Status_Processing);

    EXPECT_EQ(signalCounter.txCount      , 2); // Tx signal should be emitted for 2nd client because 1st client's read() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 1); // Rx signal should be emitted because read() returned Good for the first client
    EXPECT_EQ(signalCounter.completeCount, 1); // Complete signal should be emitted because 1st client's operation is complete

    EXPECT_EQ(clientPort.currentClient(), &client2); // Second client should be current client

    // -----------------------------------------------------------------------------
    // Step 3. 2nd client's read() returns Good, so 3rd client becomes current client
    status1 = client1.readHoldingRegisters(0, 2, readValues);
    status2 = client2.readHoldingRegisters(0, 2, readValues);
    status3 = client3.readHoldingRegisters(0, 2, readValues);

    EXPECT_EQ(status1, Status_Processing); // First client's read() should return Processing because it's already completed and current client is now 2nd client
    EXPECT_EQ(status2, Status_Good      ); // Second client's read() should return Good because it's current client and its operation is complete
    EXPECT_EQ(status3, Status_Processing);

    EXPECT_EQ(signalCounter.txCount      , 3); // Tx signal should be emitted for 3rd client because 2nd client's read() returned Good
    EXPECT_EQ(signalCounter.rxCount      , 2); // Rx signal should be emitted because read() returned Good for the 2nd client
    EXPECT_EQ(signalCounter.completeCount, 2); // Complete signal should be emitted because 2nd client's operation is complete

    EXPECT_EQ(clientPort.currentClient(), &client3); // Third client should be current client

    // -----------------------------------------------------------------------------
    // Step 4. 3rd client's read() returns Good, so all clients have completed their operations
    status1 = client1.readHoldingRegisters(0, 2, readValues);
    status2 = client2.readHoldingRegisters(0, 2, readValues);
    status3 = client3.readHoldingRegisters(0, 2, readValues);

    EXPECT_EQ(status1, Status_Processing); // First client's read() should return Processing because it's already completed and current client is now 3rd client
    EXPECT_EQ(status2, Status_Processing); // Second client's read() should return Processing because it's already completed and current client is now 3rd client
    EXPECT_EQ(status3, Status_Good      ); // Third client's read() should return Good because it's current client and its operation is complete

    EXPECT_EQ(signalCounter.txCount      , 3); // Tx signal should not be emitted for 3rd client because it was already emitted for 3rd client in previous step
    EXPECT_EQ(signalCounter.rxCount      , 3); // Rx signal should be emitted because read() returned Good for the 3rd client
    EXPECT_EQ(signalCounter.completeCount, 3); // Complete signal should be emitted because 3rd client's operation is complete

    EXPECT_EQ(clientPort.currentClient(), nullptr); // Current client should be nullptr because all clients have completed their operations
}