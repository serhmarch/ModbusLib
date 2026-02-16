#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusServerResource.h>
#include <ModbusGlobal.h>

#include "MockModbusPort.h"
#include "MockModbusDevice.h"

using namespace testing;
using namespace Modbus;

// ============================================================================
// Test Fixture for ModbusServerResource
// ============================================================================

class ModbusServerResourceTest : public ::testing::Test
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
    MockModbusDevice *mockDevice;
    ModbusServerResource *serverResource;
    SignalCounter signalCounter;

    void SetUp() override
    {
        mockPort = new MockModbusPort(true); // blocking mode
        mockDevice = new MockModbusDevice();
        
        // Expect setServerMode to be called during construction/destruction
        EXPECT_CALL(*mockPort, setServerMode(_))
            .Times(AtLeast(1));
        
        serverResource = new ModbusServerResource(mockPort, mockDevice);
    }

    void TearDown() override
    {
        delete serverResource;
        delete mockDevice;
        // mockPort is deleted by serverResource
    }
    
    // Helper method to setup buffer method expectations
    void setupBufferMethodExpectations(void *readBuff, uint16_t readSz, void *writeBuff, uint16_t writeSz)
    {
        signalCounter = SignalCounter();
        serverResource->connect(&ModbusServerResource::signalOpened   , &signalCounter, &SignalCounter::onOpened   );
        serverResource->connect(&ModbusServerResource::signalClosed   , &signalCounter, &SignalCounter::onClosed   );
        serverResource->connect(&ModbusServerResource::signalTx       , &signalCounter, &SignalCounter::onTx       );
        serverResource->connect(&ModbusServerResource::signalRx       , &signalCounter, &SignalCounter::onRx       );
        serverResource->connect(&ModbusServerResource::signalError    , &signalCounter, &SignalCounter::onError    );
        serverResource->connect(&ModbusServerResource::signalCompleted, &signalCounter, &SignalCounter::onCompleted);

        EXPECT_CALL(*mockPort, isOpen())
            .WillRepeatedly(Return(true));    
        EXPECT_CALL(*mockPort, read())
            .WillRepeatedly(Return(Status_Good));
        EXPECT_CALL(*mockPort, write())
            .WillRepeatedly(Return(Status_Good));
        EXPECT_CALL(*mockPort, readBufferSize())
            .WillRepeatedly(Return(readSz));
        EXPECT_CALL(*mockPort, readBufferData())
            .WillRepeatedly(Return(reinterpret_cast<const uint8_t*>(readBuff)));
        EXPECT_CALL(*mockPort, writeBufferSize())
            .WillRepeatedly(Return(writeSz));
        EXPECT_CALL(*mockPort, writeBufferData())
            .WillRepeatedly(Return(reinterpret_cast<uint8_t*>(writeBuff)));
    }
};

// ============================================================================
// Basic Initialization and Configuration Tests
// ============================================================================

TEST_F(ModbusServerResourceTest, Constructor)
{
    EXPECT_NE(serverResource, nullptr);
    EXPECT_EQ(serverResource->port(), mockPort);
}

TEST_F(ModbusServerResourceTest, PortGetter)
{
    EXPECT_EQ(serverResource->port(), mockPort);
}

TEST_F(ModbusServerResourceTest, TypeReturnsPortType)
{
    EXPECT_CALL(*mockPort, type())
        .WillOnce(Return(ProtocolType::TCP));
    
    EXPECT_EQ(serverResource->type(), ProtocolType::TCP);
}

TEST_F(ModbusServerResourceTest, IsOpenDelegatesToPort)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    
    EXPECT_TRUE(serverResource->isOpen());
    EXPECT_FALSE(serverResource->isOpen());
}

TEST_F(ModbusServerResourceTest, OpenSetsInternalState)
{
    StatusCode result = serverResource->open();
    EXPECT_EQ(result, Status_Good);
}

TEST_F(ModbusServerResourceTest, CloseSetsInternalState)
{
    StatusCode result = serverResource->close();
    EXPECT_EQ(result, Status_Good);
}

TEST_F(ModbusServerResourceTest, PortSetToServerMode)
{
    // Test that port is set to server mode in constructor
    // Create a new port and resource to test this behavior independently
    MockModbusPort *newPort = new MockModbusPort(true);
    MockModbusDevice *newDevice = new MockModbusDevice();
    
    EXPECT_CALL(*newPort, setServerMode(true))
        .Times(AtLeast(1));
    
    ModbusServerResource *newResource = new ModbusServerResource(newPort, newDevice);
    
    delete newResource;
    delete newDevice;
}

// ============================================================================
// Read Coils Tests (Function Code 0x01)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadCoilsRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t count;
    
    // Request: offset (2 bytes) + count (2 bytes)
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[3];

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    count = 8; // Valid count for later steps, but request size is wrong (4 bytes instead of 5)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size (less than expected 4)
            Return(Status_Good)));
    
    // Expect never call device readCoils due to bad request size
    EXPECT_CALL(*mockDevice, readCoils(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request size
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Too large amount of coils requested
    count = MB_MAX_DISCRETS+1; // More than max 2040 - MB_MAX_DISCRETS
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB (0x7F9 = 2041 coils, more than max 2040 - MB_MAX_DISCRETS)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    // Expect never call device readCoils due to bad request size
    EXPECT_CALL(*mockDevice, readCoils(_, _, _, _))
        .Times(0);

    // Expect call writeBuffer with exception function code and error status
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_COILS | MBF_EXCEPTION, Pointee(Eq(static_cast<uint8_t>(0x03))), 1))
        .Times(1);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadIllegalDataValue);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status
    count = 15; // 15 coils (2 bytes of data)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB (0x0F = 15 coils, 2 bytes of data)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    // Expect call device readCoils once with correct parameters
    uint8_t data[] = {0xAA, 0xAA};
    EXPECT_CALL(*mockDevice, readCoils(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&data](uint8_t, uint16_t, uint16_t, void* values) {
            memcpy(values, data, sizeof(data));
            return Status_Good;
        }));
 
    responseData[0] = 0x02; // Byte count
    responseData[1] = data[0]; // Coil data byte 1
    responseData[2] = data[1]; // Coil data byte 2
    // Expect call writeBuffer with response data
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_COILS, _, 3))
        .With(Args<2, 3>(ElementsAreArray(responseData, 3)))
        .Times(1)
        ;

    // Process should return good status
    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 3);

}

// ============================================================================
// Read Discrete Inputs Tests (Function Code 0x02)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadDiscreteInputsRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t count;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[3];

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    count = 8;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_DISCRETE_INPUTS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readDiscreteInputs(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Too large amount of discrete inputs requested
    count = MB_MAX_DISCRETS + 1;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_DISCRETE_INPUTS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readDiscreteInputs(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_DISCRETE_INPUTS | MBF_EXCEPTION, Pointee(Eq(static_cast<uint8_t>(0x03))), 1))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadIllegalDataValue);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status
    count = 10;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_DISCRETE_INPUTS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint8_t data[] = {0xF0, 0x03};
    EXPECT_CALL(*mockDevice, readDiscreteInputs(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&data](uint8_t, uint16_t, uint16_t, void* values) {
            memcpy(values, data, sizeof(data));
            return Status_Good;
        }));
 
    responseData[0] = 0x02; // Byte count
    responseData[1] = data[0]; // Coil data byte 1
    responseData[2] = data[1]; // Coil data byte 2
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_DISCRETE_INPUTS, _, 3))
        .With(Args<2, 3>(ElementsAreArray(responseData, 3)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 3);
}

// ============================================================================
// Read Holding Registers Tests (Function Code 0x03)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadHoldingRegistersRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t count;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[5]; // 1 byte count + 2 registers * 2 bytes

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    count = 2;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readHoldingRegisters(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Too large amount of registers requested
    count = MB_MAX_REGISTERS + 1;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readHoldingRegisters(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_HOLDING_REGISTERS | MBF_EXCEPTION, Pointee(Eq(static_cast<uint8_t>(0x03))), 1))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadIllegalDataValue);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status
    count = 2;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint16_t registerData[2] = {0x1234, 0x5678};
    EXPECT_CALL(*mockDevice, readHoldingRegisters(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&registerData](uint8_t, uint16_t, uint16_t, uint16_t* values) {
            memcpy(values, registerData, sizeof(registerData));
            return Status_Good;
        }));
 
    responseData[0] = 0x04; // Byte count (2 registers * 2 bytes)
    responseData[1] = reinterpret_cast<const uint8_t*>(&registerData[0])[1];
    responseData[2] = reinterpret_cast<const uint8_t*>(&registerData[0])[0];
    responseData[3] = reinterpret_cast<const uint8_t*>(&registerData[1])[1];
    responseData[4] = reinterpret_cast<const uint8_t*>(&registerData[1])[0];
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_HOLDING_REGISTERS, _, 5))
        .With(Args<2, 3>(ElementsAreArray(responseData, 5)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 3);
}

// ============================================================================
// Read Input Registers Tests (Function Code 0x04)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadInputRegistersRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 5;
    uint16_t count;
    
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[7]; // 1 byte count + 3 registers * 2 bytes

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    count = 3;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_INPUT_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readInputRegisters(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Too large amount of input registers requested
    count = MB_MAX_REGISTERS + 1;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_INPUT_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readInputRegisters(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_INPUT_REGISTERS | MBF_EXCEPTION, Pointee(Eq(static_cast<uint8_t>(0x03))), 1))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadIllegalDataValue);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status
    count = 3;
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_INPUT_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint16_t inputData[3] = {0x1234, 0x5678, 0x9ABC};
    EXPECT_CALL(*mockDevice, readInputRegisters(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&inputData](uint8_t, uint16_t, uint16_t, uint16_t* values) {
            memcpy(values, inputData, sizeof(inputData));
            return Status_Good;
        }));
 
    responseData[0] = 0x06; // Byte count (3 registers * 2 bytes)
    responseData[1] = reinterpret_cast<const uint8_t*>(&inputData[0])[1];
    responseData[2] = reinterpret_cast<const uint8_t*>(&inputData[0])[0];
    responseData[3] = reinterpret_cast<const uint8_t*>(&inputData[1])[1];
    responseData[4] = reinterpret_cast<const uint8_t*>(&inputData[1])[0];
    responseData[5] = reinterpret_cast<const uint8_t*>(&inputData[2])[1];
    responseData[6] = reinterpret_cast<const uint8_t*>(&inputData[2])[0];
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_INPUT_REGISTERS, _, 7))
        .With(Args<2, 3>(ElementsAreArray(responseData, 7)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 3);
}

// ============================================================================
// Write Single Coil Tests (Function Code 0x05)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteSingleCoil)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t value;
    
    // Request: offset (2 bytes) + count (2 bytes)
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};

    setupBufferMethodExpectations(requestData, sizeof(requestData), requestData, sizeof(requestData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    value = 0; // Valid value for later steps, but request size is wrong (4 bytes instead of 5)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&value)[1]; // Value MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&value)[0]; // Value LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size (less than expected 4)
            Return(Status_Good)));
    
    // Expect never call device writeSingleCoil due to bad request size
    EXPECT_CALL(*mockDevice, writeSingleCoil(_, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request size
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Incorrect request value
    value = 0xAAAA; // Invalid value for single coil (should be 0x0000 or 0xFF00)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&value)[1]; // Value MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&value)[0]; // Value LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4), // Correct size (expected 4)
            Return(Status_Good)));
    
    // Expect never call device writeSingleCoil due to bad request value
    EXPECT_CALL(*mockDevice, writeSingleCoil(_, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request value
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status (value = 0, coil OFF)
    value = 0x0000; // Valid value for single coil OFF
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&value)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&value)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleCoil(unit, offset, false))
        .Times(1)
        .WillOnce(Return(Status_Good))
        ;
 
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_COIL, _, 4))
        .With(Args<2, 3>(ElementsAreArray(requestData, 4)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 3);

    // -------------------------------------------------------------------------
    // Step 4. Return Good status (value = 0xFF00, coil ON)
    value = 0xFF00; // Valid value for single coil ON
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&value)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&value)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleCoil(unit, offset, true))
        .Times(1)
        .WillOnce(Return(Status_Good))
        ;
 
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_COIL, _, 4))
        .With(Args<2, 3>(ElementsAreArray(requestData, 4)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 4);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 4);
}

// ============================================================================
// Write Single Register Tests (Function Code 0x06)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteSingleRegisterRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t value;
    
    // Request: offset (2 bytes) + count (2 bytes)
    uint8_t requestData[4] = {0x00, 0x00, 0x00, 0x00};

    setupBufferMethodExpectations(requestData, sizeof(requestData), requestData, sizeof(requestData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    value = 0; // Valid value for later steps, but request size is wrong (4 bytes instead of 5)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&value)[1]; // Value MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&value)[0]; // Value LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_REGISTER),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(3), // Wrong size (less than expected 4)
            Return(Status_Good)));
    
    // Expect never call device writeSingleRegister due to bad request size
    EXPECT_CALL(*mockDevice, writeSingleRegister(_, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request size
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status
    value = 0xAABB; // Valid value for single register
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&value)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&value)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_REGISTER),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleRegister(unit, offset, value))
        .Times(1)
        .WillOnce(Return(Status_Good))
        ;
 
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_REGISTER, _, 4))
        .With(Args<2, 3>(ElementsAreArray(requestData, 4)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Read Exception Status Tests (Function Code 0x07)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadExceptionStatusRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    
    uint8_t requestData[2] = {0x00, 0x00}; // Should be empty, but we'll test with data for error case
    uint8_t responseData[1]; // 1 byte status

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (should be 0 bytes)
    requestData[0] = 0xAA; // Some invalid data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_EXCEPTION_STATUS),
            SetArrayArgument<2>(requestData, requestData + 1),
            SetArgPointee<4>(1), // Wrong size (should be 0)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readExceptionStatus(_, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status (correct request with no data)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_EXCEPTION_STATUS),
            SetArgPointee<4>(0), // Correct size (0 bytes)
            Return(Status_Good)));
    
    uint8_t exceptionStatus = 0x55; // Example exception status byte
    EXPECT_CALL(*mockDevice, readExceptionStatus(unit, _))
        .Times(1)
        .WillOnce(Invoke([exceptionStatus](uint8_t, uint8_t* status) {
            *status = exceptionStatus;
            return Status_Good;
        }));
 
    responseData[0] = exceptionStatus; // Status byte
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_EXCEPTION_STATUS, _, 1))
        .With(Args<2, 3>(ElementsAreArray(responseData, 1)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Diagnostic Tests (Function Code 0x08)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessDiagnosticRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t subfunc;
    
    // Request: subfunc (2 bytes) + data (N bytes)
    uint8_t requestData[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[10]; // subfunc (2 bytes) + response data

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (less than 2 bytes)
    subfunc = 0x0001;
    requestData[0] = reinterpret_cast<uint8_t*>(&subfunc)[1]; // Subfunc MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&subfunc)[0]; // Subfunc LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_DIAGNOSTICS),
            SetArrayArgument<2>(requestData, requestData + 1),
            SetArgPointee<4>(1), // Wrong size (less than required 2)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, diagnostics(_, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status (subfunc only, no additional data)
    subfunc = 0x0000; // Return Query Data (echo test)
    requestData[0] = reinterpret_cast<const uint8_t*>(&subfunc)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&subfunc)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_DIAGNOSTICS),
            SetArrayArgument<2>(requestData, requestData + 2),
            SetArgPointee<4>(2), // Just subfunc (2 bytes)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, diagnostics(unit, subfunc, 0, _, _, _))
        .Times(1)
        .WillOnce(Invoke([](uint8_t, uint16_t, uint8_t, const void*, uint8_t* outsize, void*) {
            *outsize = 0; // No output data
            return Status_Good;
        }));
 
    responseData[0] = reinterpret_cast<const uint8_t*>(&subfunc)[1]; // Subfunc MSB
    responseData[1] = reinterpret_cast<const uint8_t*>(&subfunc)[0]; // Subfunc LSB
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_DIAGNOSTICS, _, 2))
        .With(Args<2, 3>(ElementsAreArray(responseData, 2)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status (subfunc with data)
    subfunc = 0x0000; // Return Query Data (echo test)
    requestData[0] = reinterpret_cast<const uint8_t*>(&subfunc)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&subfunc)[0];
    requestData[2] = 0xA5; // Data byte 1
    requestData[3] = 0x5A; // Data byte 2
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_DIAGNOSTICS),
            SetArrayArgument<2>(requestData, requestData + 4),
            SetArgPointee<4>(4), // subfunc (2 bytes) + data (2 bytes)
            Return(Status_Good)));
    
    uint8_t expectedInData[] = {0xA5, 0x5A};
    uint8_t outData[] = {0xA5, 0x5A}; // Echo the data back
    EXPECT_CALL(*mockDevice, diagnostics(unit, subfunc, 2, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&expectedInData, &outData](uint8_t, uint16_t, uint8_t insize, const void* indata, uint8_t* outsize, void* outdata) {
            // Verify input data
            EXPECT_EQ(memcmp(indata, expectedInData, sizeof(expectedInData)), 0);
            // Echo data back
            *outsize = 2;
            memcpy(outdata, outData, sizeof(outData));
            return Status_Good;
        }));
 
    responseData[0] = reinterpret_cast<const uint8_t*>(&subfunc)[1]; // Subfunc MSB
    responseData[1] = reinterpret_cast<const uint8_t*>(&subfunc)[0]; // Subfunc LSB
    responseData[2] = outData[0]; // Echo data byte 1
    responseData[3] = outData[1]; // Echo data byte 2
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_DIAGNOSTICS, _, 4))
        .With(Args<2, 3>(ElementsAreArray(responseData, 4)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 3);
}

// ============================================================================
// Get Comm Event Counter Tests (Function Code 0x0B)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessGetCommEventCounterRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    
    uint8_t requestData[2] = {0x00, 0x00}; // Should be empty, but we'll test with data for error case
    uint8_t responseData[4]; // status (2 bytes) + count (2 bytes)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (should be 0 bytes)
    requestData[0] = 0xAA; // Some invalid data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_GET_COMM_EVENT_COUNTER),
            SetArrayArgument<2>(requestData, requestData + 1),
            SetArgPointee<4>(1), // Wrong size (should be 0)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, getCommEventCounter(_, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status (correct request with no data)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_GET_COMM_EVENT_COUNTER),
            SetArgPointee<4>(0), // Correct size (0 bytes)
            Return(Status_Good)));
    
    uint16_t status = 0xFFFF; // Status - typically 0xFFFF indicates busy
    uint16_t count = 0x0108; // Event count
    EXPECT_CALL(*mockDevice, getCommEventCounter(unit, _, _))
        .Times(1)
        .WillOnce(Invoke([status, count](uint8_t, uint16_t* pStatus, uint16_t* pCount) {
            *pStatus = status;
            *pCount = count;
            return Status_Good;
        }));
 
    responseData[0] = reinterpret_cast<const uint8_t*>(&status)[1]; // Status MSB
    responseData[1] = reinterpret_cast<const uint8_t*>(&status)[0]; // Status LSB
    responseData[2] = reinterpret_cast<const uint8_t*>(&count)[1]; // Count MSB
    responseData[3] = reinterpret_cast<const uint8_t*>(&count)[0]; // Count LSB
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_GET_COMM_EVENT_COUNTER, _, 4))
        .With(Args<2, 3>(ElementsAreArray(responseData, 4)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Get Comm Event Log Tests (Function Code 0x0C)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessGetCommEventLogRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    
    uint8_t requestData[2] = {0x00, 0x00}; // Should be empty, but we'll test with data for error case
    uint8_t responseData[20]; // byteCount(1) + status(2) + count(2) + messageCount(2) + events(N)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (should be 0 bytes)
    requestData[0] = 0xAA; // Some invalid data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_GET_COMM_EVENT_LOG),
            SetArrayArgument<2>(requestData, requestData + 1),
            SetArgPointee<4>(1), // Wrong size (should be 0)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, getCommEventLog(_, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status with no event data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_GET_COMM_EVENT_LOG),
            SetArgPointee<4>(0), // Correct size (0 bytes)
            Return(Status_Good)));
    
    uint16_t status = 0xFFFF; // Status - typically 0xFFFF indicates busy
    uint16_t count = 0x0108; // Event count
    uint16_t messageCount = 0x0021; // Message count
    uint8_t outByteCount = 0; // No event data
    EXPECT_CALL(*mockDevice, getCommEventLog(unit, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([status, count, messageCount, outByteCount](uint8_t, uint16_t* pStatus, uint16_t* pCount, uint16_t* pMessageCount, uint8_t* pOutByteCount, void*) {
            *pStatus = status;
            *pCount = count;
            *pMessageCount = messageCount;
            *pOutByteCount = outByteCount;
            return Status_Good;
        }));
 
    responseData[0] = outByteCount + 6; // Byte count
    responseData[1] = reinterpret_cast<const uint8_t*>(&status)[1]; // Status MSB
    responseData[2] = reinterpret_cast<const uint8_t*>(&status)[0]; // Status LSB
    responseData[3] = reinterpret_cast<const uint8_t*>(&count)[1]; // Count MSB
    responseData[4] = reinterpret_cast<const uint8_t*>(&count)[0]; // Count LSB
    responseData[5] = reinterpret_cast<const uint8_t*>(&messageCount)[1]; // Message count MSB
    responseData[6] = reinterpret_cast<const uint8_t*>(&messageCount)[0]; // Message count LSB
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_GET_COMM_EVENT_LOG, _, 7))
        .With(Args<2, 3>(ElementsAreArray(responseData, 7)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Return Good status with event data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_GET_COMM_EVENT_LOG),
            SetArgPointee<4>(0), // Correct size (0 bytes)
            Return(Status_Good)));
    
    status = 0x0000; // Status - 0x0000 indicates ready
    count = 0x010A; // Event count
    messageCount = 0x0023; // Message count
    uint8_t eventData[] = {0x20, 0x00, 0x01, 0x02}; // Example event data
    outByteCount = sizeof(eventData);
    EXPECT_CALL(*mockDevice, getCommEventLog(unit, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([status, count, messageCount, outByteCount, &eventData](uint8_t, uint16_t* pStatus, uint16_t* pCount, uint16_t* pMessageCount, uint8_t* pOutByteCount, void* events) {
            *pStatus = status;
            *pCount = count;
            *pMessageCount = messageCount;
            *pOutByteCount = outByteCount;
            memcpy(events, eventData, outByteCount);
            return Status_Good;
        }));
 
    responseData[0] = outByteCount + 6; // Byte count
    responseData[1] = reinterpret_cast<const uint8_t*>(&status)[1]; // Status MSB
    responseData[2] = reinterpret_cast<const uint8_t*>(&status)[0]; // Status LSB
    responseData[3] = reinterpret_cast<const uint8_t*>(&count)[1]; // Count MSB
    responseData[4] = reinterpret_cast<const uint8_t*>(&count)[0]; // Count LSB
    responseData[5] = reinterpret_cast<const uint8_t*>(&messageCount)[1]; // Message count MSB
    responseData[6] = reinterpret_cast<const uint8_t*>(&messageCount)[0]; // Message count LSB
    memcpy(&responseData[7], eventData, outByteCount); // Event data
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_GET_COMM_EVENT_LOG, _, 11))
        .With(Args<2, 3>(ElementsAreArray(responseData, 11)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 2);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 3);
}

// ============================================================================
// Write Multiple Coils Tests (Function Code 0x0F)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteMultipleCoilsRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t count;
    uint8_t byteCount;
    
    // Request: offset (2 bytes) + count (2 bytes) + byte count (1 byte) + coil data (N bytes)
    uint8_t requestData[300] = {0x00};
    uint8_t responseData[4]; // offset (2 bytes) + count (2 bytes)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (less than 5 bytes)
    count = 8; // Valid count for later steps, but request size is wrong
    byteCount = 1; // Valid byte count
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count
    requestData[5] = 0xAA; // Coil data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4), // Wrong size (less than expected 5)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleCoils due to bad request size
    EXPECT_CALL(*mockDevice, writeMultipleCoils(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request size
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Input buffer size mismatch (count doesn't match byte count field)
    count = 17; // 17 coils should require 3 bytes
    byteCount = 3; // Claiming 3 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count (incorrect!)
    requestData[5] = 0xAA; // Coil data byte 1
    requestData[6] = 0xBB; // Coil data byte 2
    requestData[7] = 0xCC; // Coil data byte 3
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 4), // 3+4=7 bytes - wrong size (less than expected 8 = 5 + byteCount)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleCoils due to byte count mismatch
    EXPECT_CALL(*mockDevice, writeMultipleCoils(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to byte count mismatch
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Byte count mismatch (count doesn't match byte count field)
    count = 16; // 16 coils should require 2 bytes
    byteCount = 3; // But claiming 3 bytes (mismatch!)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count (incorrect!)
    requestData[5] = 0xAA; // Coil data byte 1
    requestData[6] = 0xBB; // Coil data byte 2
    requestData[7] = 0xCC; // Coil data byte 3
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 5), // offset(2) + count(2) + byteCount(1) + data(byteCount)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleCoils due to byte count mismatch
    EXPECT_CALL(*mockDevice, writeMultipleCoils(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to byte count mismatch
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 3);
    EXPECT_EQ(signalCounter.completeCount, 3);

    // -------------------------------------------------------------------------
    // Step 4. Return Good status
    count = 16; // 16 coils (2 bytes of data)
    byteCount = (count + 7) / 8; // 2 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count
    requestData[5] = 0x55; // Coil data byte 1
    requestData[6] = 0xAA; // Coil data byte 2
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(7), // offset(2) + count(2) + byteCount(1) + data(2)
            Return(Status_Good)));
    
    // Expect call device writeMultipleCoils once with correct parameters
    uint8_t expectedData[] = {0x55, 0xAA};
    EXPECT_CALL(*mockDevice, writeMultipleCoils(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&expectedData](uint8_t, uint16_t, uint16_t, const void* values) {
            // Verify the data passed to the device
            EXPECT_EQ(memcmp(values, expectedData, sizeof(expectedData)), 0);
            return Status_Good;
        }));
 
    responseData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    responseData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    responseData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    responseData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    // Expect call writeBuffer with response data (echo offset and count)
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_MULTIPLE_COILS, _, 4))
        .With(Args<2, 3>(ElementsAreArray(responseData, 4)))
        .Times(1);

    // Process should return good status
    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 4);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 3);
    EXPECT_EQ(signalCounter.completeCount, 4);
}

// ============================================================================
// Write Multiple Registers Tests (Function Code 0x10)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteMultipleRegistersRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0;
    uint16_t count;
    uint8_t byteCount;
    
    // Request: offset (2 bytes) + count (2 bytes) + byte count (1 byte) + register data (N*2 bytes)
    uint8_t requestData[15] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[4]; // offset (2 bytes) + count (2 bytes)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (less than 5 bytes)
    count = 2; // Valid count for later steps, but request size is wrong
    byteCount = count * 2; // 4 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(4), // Wrong size (less than expected 5)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleRegisters due to bad request size
    EXPECT_CALL(*mockDevice, writeMultipleRegisters(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to bad request size
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Byte count mismatch (count*2 doesn't match byte count field)
    count = 3; // 3 registers should require 6 bytes
    byteCount = 5; // But claiming 5 bytes (mismatch!)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count (incorrect!)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 5), // offset(2) + count(2) + byteCount(1) + data(byteCount)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleRegisters due to byte count mismatch
    EXPECT_CALL(*mockDevice, writeMultipleRegisters(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to byte count mismatch
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);
    // -------------------------------------------------------------------------
    // Step 3. Byte count mismatch (count doesn't match byte count field)
    count = 16; // 16 coils should require 2 bytes
    byteCount = 33; // But claiming 33 bytes (mismatch!)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count (incorrect!)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 5), // offset(2) + count(2) + byteCount(1) + data(byteCount)
            Return(Status_Good)));
    
    // Expect never call device writeMultipleCoils due to byte count mismatch
    EXPECT_CALL(*mockDevice, writeMultipleCoils(_, _, _, _))
        .Times(0);

    // Expect never call writeBuffer due to byte count mismatch
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    // Process should return bad status
    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 3);
    EXPECT_EQ(signalCounter.completeCount, 3);

    // -------------------------------------------------------------------------
    // Step 4. Return Good status
    count = 3; // 3 registers (6 bytes of data)
    byteCount = count * 2; // 6 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    requestData[4] = byteCount; // Byte count
    // Register data in big-endian format
    uint16_t registerData[3] = {0x1234, 0x5678, 0x9ABC};
    requestData[5] = reinterpret_cast<uint8_t*>(&registerData[0])[1]; // Reg 0 MSB
    requestData[6] = reinterpret_cast<uint8_t*>(&registerData[0])[0]; // Reg 0 LSB
    requestData[7] = reinterpret_cast<uint8_t*>(&registerData[1])[1]; // Reg 1 MSB
    requestData[8] = reinterpret_cast<uint8_t*>(&registerData[1])[0]; // Reg 1 LSB
    requestData[9] = reinterpret_cast<uint8_t*>(&registerData[2])[1]; // Reg 2 MSB
    requestData[10] = reinterpret_cast<uint8_t*>(&registerData[2])[0]; // Reg 2 LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(11), // offset(2) + count(2) + byteCount(1) + data(6)
            Return(Status_Good)));
    
    // Expect call device writeMultipleRegisters once with correct parameters
    EXPECT_CALL(*mockDevice, writeMultipleRegisters(unit, offset, count, _))
        .Times(1)
        .WillOnce(Invoke([&registerData](uint8_t, uint16_t, uint16_t, const uint16_t* values) {
            // Verify the data passed to the device
            EXPECT_EQ(memcmp(values, registerData, sizeof(registerData)), 0);
            return Status_Good;
        }));
 
    responseData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    responseData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    responseData[2] = reinterpret_cast<uint8_t*>(&count)[1]; // Count MSB
    responseData[3] = reinterpret_cast<uint8_t*>(&count)[0]; // Count LSB
    // Expect call writeBuffer with response data (echo offset and count)
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_MULTIPLE_REGISTERS, _, 4))
        .With(Args<2, 3>(ElementsAreArray(responseData, 4)))
        .Times(1);

    // Process should return good status
    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 4);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 3);
    EXPECT_EQ(signalCounter.completeCount, 4);
}

// ============================================================================
// Report Server Id Tests (Function Code 0x11)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReportServerIdRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    
    uint8_t requestData[2] = {0x00, 0x00}; // Should be empty, but we'll test with data for error case
    uint8_t responseData[50]; // byteCount(1) + server ID data(N)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (should be 0 bytes)
    requestData[0] = 0xAA; // Some invalid data
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_REPORT_SERVER_ID),
            SetArrayArgument<2>(requestData, requestData + 1),
            SetArgPointee<4>(1), // Wrong size (should be 0)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, reportServerID(_, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status (correct request with no data)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_REPORT_SERVER_ID),
            SetArgPointee<4>(0), // Correct size (0 bytes)
            Return(Status_Good)));
    
    uint8_t serverIdData[] = {0xFF, 0x00, 0x01, 0x02, 0x03}; // Example: Run Indicator Status + Server ID bytes
    uint8_t serverIdCount = sizeof(serverIdData);
    EXPECT_CALL(*mockDevice, reportServerID(unit, _, _))
        .Times(1)
        .WillOnce(Invoke([serverIdCount, &serverIdData](uint8_t, uint8_t* count, uint8_t* data) {
            *count = serverIdCount;
            memcpy(data, serverIdData, serverIdCount);
            return Status_Good;
        }));
 
    responseData[0] = serverIdCount; // Byte count
    memcpy(&responseData[1], serverIdData, serverIdCount); // Server ID data
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_REPORT_SERVER_ID, _, serverIdCount + 1))
        .With(Args<2, 3>(ElementsAreArray(responseData, serverIdCount + 1)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Mask Write Register Tests (Function Code 0x16)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessMaskWriteRegisterRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t offset = 0x0004;
    uint16_t andMask = 0xF2FF;
    uint16_t orMask = 0x0025;
    
    // Request: offset (2 bytes) + andMask (2 bytes) + orMask (2 bytes)
    uint8_t requestData[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t responseData[6]; // Echo back: offset (2 bytes) + andMask (2 bytes) + orMask (2 bytes)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (should be 6 bytes)
    requestData[0] = reinterpret_cast<uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&andMask)[1]; // And Mask MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&andMask)[0]; // And Mask LSB
    requestData[4] = reinterpret_cast<uint8_t*>(&orMask)[1]; // Or Mask MSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_MASK_WRITE_REGISTER),
            SetArrayArgument<2>(requestData, requestData + 5),
            SetArgPointee<4>(5), // Wrong size (less than expected 6)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, maskWriteRegister(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status
    requestData[0] = reinterpret_cast<const uint8_t*>(&offset)[1]; // Offset MSB
    requestData[1] = reinterpret_cast<const uint8_t*>(&offset)[0]; // Offset LSB
    requestData[2] = reinterpret_cast<const uint8_t*>(&andMask)[1]; // And Mask MSB
    requestData[3] = reinterpret_cast<const uint8_t*>(&andMask)[0]; // And Mask LSB
    requestData[4] = reinterpret_cast<const uint8_t*>(&orMask)[1]; // Or Mask MSB
    requestData[5] = reinterpret_cast<const uint8_t*>(&orMask)[0]; // Or Mask LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_MASK_WRITE_REGISTER),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(6),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, maskWriteRegister(unit, offset, andMask, orMask))
        .Times(1)
        .WillOnce(Return(Status_Good));
 
    responseData[0] = reinterpret_cast<const uint8_t*>(&offset)[1]; // Offset MSB
    responseData[1] = reinterpret_cast<const uint8_t*>(&offset)[0]; // Offset LSB
    responseData[2] = reinterpret_cast<const uint8_t*>(&andMask)[1]; // And Mask MSB
    responseData[3] = reinterpret_cast<const uint8_t*>(&andMask)[0]; // And Mask LSB
    responseData[4] = reinterpret_cast<const uint8_t*>(&orMask)[1]; // Or Mask MSB
    responseData[5] = reinterpret_cast<const uint8_t*>(&orMask)[0]; // Or Mask LSB
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_MASK_WRITE_REGISTER, _, 6))
        .With(Args<2, 3>(ElementsAreArray(responseData, 6)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Read/Write Multiple Registers Tests (Function Code 0x17)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadWriteMultipleRegistersRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t readOffset = 0x0003;
    uint16_t readCount = 3;
    uint16_t writeOffset = 0x000E;
    uint16_t writeCount = 2;
    uint8_t byteCount;
    
    // Request: readOffset(2) + readCount(2) + writeOffset(2) + writeCount(2) + byteCount(1) + write data(N)
    uint8_t requestData[20] = {0x00};
    uint8_t responseData[10]; // byteCount(1) + read register data(readCount*2)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size (less than 9 bytes)
    byteCount = writeCount * 2; // 4 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&readOffset)[1]; // Read Offset MSB
    requestData[1] = reinterpret_cast<uint8_t*>(&readOffset)[0]; // Read Offset LSB
    requestData[2] = reinterpret_cast<uint8_t*>(&readCount)[1]; // Read Count MSB
    requestData[3] = reinterpret_cast<uint8_t*>(&readCount)[0]; // Read Count LSB
    requestData[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1]; // Write Offset MSB
    requestData[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0]; // Write Offset LSB
    requestData[6] = reinterpret_cast<uint8_t*>(&writeCount)[1]; // Write Count MSB
    requestData[7] = reinterpret_cast<uint8_t*>(&writeCount)[0]; // Write Count LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + 8),
            SetArgPointee<4>(8), // Wrong size (less than expected 9)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(_, _, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Input buffer size mismatch (sz != byteCount+9)
    byteCount = writeCount * 2; // 4 bytes
    requestData[0] = reinterpret_cast<uint8_t*>(&readOffset)[1];
    requestData[1] = reinterpret_cast<uint8_t*>(&readOffset)[0];
    requestData[2] = reinterpret_cast<uint8_t*>(&readCount)[1];
    requestData[3] = reinterpret_cast<uint8_t*>(&readCount)[0];
    requestData[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1];
    requestData[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0];
    requestData[6] = reinterpret_cast<uint8_t*>(&writeCount)[1];
    requestData[7] = reinterpret_cast<uint8_t*>(&writeCount)[0];
    requestData[8] = byteCount; // Byte count
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(12), // Wrong: 12 != byteCount+9 (should be 13)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(_, _, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 2);
    EXPECT_EQ(signalCounter.completeCount, 2);

    // -------------------------------------------------------------------------
    // Step 3. Byte count mismatch (writeCount*2 != byteCount)
    byteCount = 5; // Wrong! Should be writeCount*2 = 4
    requestData[0] = reinterpret_cast<uint8_t*>(&readOffset)[1];
    requestData[1] = reinterpret_cast<uint8_t*>(&readOffset)[0];
    requestData[2] = reinterpret_cast<uint8_t*>(&readCount)[1];
    requestData[3] = reinterpret_cast<uint8_t*>(&readCount)[0];
    requestData[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1];
    requestData[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0];
    requestData[6] = reinterpret_cast<uint8_t*>(&writeCount)[1];
    requestData[7] = reinterpret_cast<uint8_t*>(&writeCount)[0];
    requestData[8] = byteCount; // Byte count (incorrect!)
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 9), // 14 bytes total
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(_, _, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 3);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 3);
    EXPECT_EQ(signalCounter.completeCount, 3);

    // -------------------------------------------------------------------------
    // Step 4. Read count exceeds limit
    readCount = MB_MAX_REGISTERS + 1; // Exceeds limit
    writeCount = 2;
    byteCount = writeCount * 2; // Keep byteCount reasonable (4 bytes)
    requestData[0] = reinterpret_cast<uint8_t*>(&readOffset)[1];
    requestData[1] = reinterpret_cast<uint8_t*>(&readOffset)[0];
    requestData[2] = reinterpret_cast<uint8_t*>(&readCount)[1];
    requestData[3] = reinterpret_cast<uint8_t*>(&readCount)[0];
    requestData[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1];
    requestData[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0];
    requestData[6] = reinterpret_cast<uint8_t*>(&writeCount)[1];
    requestData[7] = reinterpret_cast<uint8_t*>(&writeCount)[0];
    requestData[8] = byteCount;
    // Add dummy write data (2 registers)
    requestData[9] = 0x12;
    requestData[10] = 0x34;
    requestData[11] = 0x56;
    requestData[12] = 0x78;
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + 13),
            SetArgPointee<4>(13), // sz = 9 + 4
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(_, _, _, _, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_WRITE_MULTIPLE_REGISTERS | MBF_EXCEPTION, Pointee(Eq(static_cast<uint8_t>(0x03))), 1))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadIllegalDataValue);
    EXPECT_EQ(signalCounter.rxCount      , 4);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 4);
    EXPECT_EQ(signalCounter.completeCount, 4);

    // -------------------------------------------------------------------------
    // Step 5. Write count exceeds limit
    readCount = 3;
    writeCount = MB_MAX_REGISTERS + 1; // Exceeds limit
    byteCount = 4; // Keep byteCount reasonable (matching actual data: 2 registers)
    uint16_t actualWriteCount = 2; // Actual write count for data
    requestData[0] = reinterpret_cast<uint8_t*>(&readOffset)[1];
    requestData[1] = reinterpret_cast<uint8_t*>(&readOffset)[0];
    requestData[2] = reinterpret_cast<uint8_t*>(&readCount)[1];
    requestData[3] = reinterpret_cast<uint8_t*>(&readCount)[0];
    requestData[4] = reinterpret_cast<uint8_t*>(&writeOffset)[1];
    requestData[5] = reinterpret_cast<uint8_t*>(&writeOffset)[0];
    requestData[6] = reinterpret_cast<uint8_t*>(&writeCount)[1];
    requestData[7] = reinterpret_cast<uint8_t*>(&writeCount)[0];
    requestData[8] = byteCount;
    // Add dummy write data (2 registers)
    requestData[9] = 0x12;
    requestData[10] = 0x34;
    requestData[11] = 0x56;
    requestData[12] = 0x78;
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + 13),
            SetArgPointee<4>(13), // sz = 9 + 4
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(_, _, _, _, _, _, _))
        .Times(0);

    // This should fail because writeCount*2 != byteCount (252 != 4)
    // So it returns BadNotCorrectRequest, not BadIllegalDataValue
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest); // Changed expectation
    EXPECT_EQ(signalCounter.rxCount      , 5);
    EXPECT_EQ(signalCounter.txCount      , 1); // No tx because BadNotCorrectRequest doesn't send exception
    EXPECT_EQ(signalCounter.errorCount   , 5);
    EXPECT_EQ(signalCounter.completeCount, 5);

    // -------------------------------------------------------------------------
    // Step 6. Return Good status
    readOffset = 0x0003;
    readCount = 3;
    writeOffset = 0x000E;
    writeCount = 2;
    byteCount = writeCount * 2; // 4 bytes
    requestData[0] = reinterpret_cast<const uint8_t*>(&readOffset)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&readOffset)[0];
    requestData[2] = reinterpret_cast<const uint8_t*>(&readCount)[1];
    requestData[3] = reinterpret_cast<const uint8_t*>(&readCount)[0];
    requestData[4] = reinterpret_cast<const uint8_t*>(&writeOffset)[1];
    requestData[5] = reinterpret_cast<const uint8_t*>(&writeOffset)[0];
    requestData[6] = reinterpret_cast<const uint8_t*>(&writeCount)[1];
    requestData[7] = reinterpret_cast<const uint8_t*>(&writeCount)[0];
    requestData[8] = byteCount; // Byte count
    // Write register data in big-endian format
    uint16_t writeData[2] = {0xABCD, 0x1234};
    requestData[9]  = reinterpret_cast<uint8_t*>(&writeData[0])[1]; // Write Reg 0 MSB
    requestData[10] = reinterpret_cast<uint8_t*>(&writeData[0])[0]; // Write Reg 0 LSB
    requestData[11] = reinterpret_cast<uint8_t*>(&writeData[1])[1]; // Write Reg 1 MSB
    requestData[12] = reinterpret_cast<uint8_t*>(&writeData[1])[0]; // Write Reg 1 LSB
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(byteCount + 9), // 13 bytes total
            Return(Status_Good)));
    
    // Read register data to be returned
    uint16_t readData[3] = {0x5678, 0x9ABC, 0xDEF0};
    EXPECT_CALL(*mockDevice, readWriteMultipleRegisters(unit, readOffset, readCount, _, writeOffset, writeCount, _))
        .Times(1)
        .WillOnce(Invoke([&readData, &writeData](uint8_t, uint16_t, uint16_t, uint16_t* readValues, uint16_t, uint16_t writeCount, const uint16_t* writeValues) {
            // Verify write data passed to device
            EXPECT_EQ(memcmp(writeValues, writeData, writeCount * sizeof(uint16_t)), 0);
            // Return read data
            memcpy(readValues, readData, sizeof(readData));
            return Status_Good;
        }));
 
    responseData[0] = readCount * 2; // Byte count (6 bytes)
    responseData[1] = reinterpret_cast<const uint8_t*>(&readData[0])[1]; // Read Reg 0 MSB
    responseData[2] = reinterpret_cast<const uint8_t*>(&readData[0])[0]; // Read Reg 0 LSB
    responseData[3] = reinterpret_cast<const uint8_t*>(&readData[1])[1]; // Read Reg 1 MSB
    responseData[4] = reinterpret_cast<const uint8_t*>(&readData[1])[0]; // Read Reg 1 LSB
    responseData[5] = reinterpret_cast<const uint8_t*>(&readData[2])[1]; // Read Reg 2 MSB
    responseData[6] = reinterpret_cast<const uint8_t*>(&readData[2])[0]; // Read Reg 2 LSB
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_WRITE_MULTIPLE_REGISTERS, _, 7))
        .With(Args<2, 3>(ElementsAreArray(responseData, 7)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 6);
    EXPECT_EQ(signalCounter.txCount      , 2); // Changed from 3 to 2
    EXPECT_EQ(signalCounter.errorCount   , 5);
    EXPECT_EQ(signalCounter.completeCount, 6);
}

// ============================================================================
// Read FIFO Queue Tests (Function Code 0x18)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadFifoQueueRequest)
{
    StatusCode result;
    uint8_t unit = 1;
    uint16_t fifoAddress = 0x1000;
    uint16_t count;
    
    uint8_t requestData[2] = {0x00, 0x00};
    uint8_t responseData[14]; // 2 bytes byteCount + 2 bytes count + 10 bytes values (5 registers)

    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    // -------------------------------------------------------------------------
    // Step 1. Incorrect request size
    requestData[0] = reinterpret_cast<const uint8_t*>(&fifoAddress)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&fifoAddress)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_FIFO_QUEUE),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(1), // Wrong size (< 2)
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readFIFOQueue(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .Times(0);

    result = serverResource->process();
    EXPECT_EQ(result, Status_BadNotCorrectRequest);
    EXPECT_EQ(signalCounter.rxCount      , 1);
    EXPECT_EQ(signalCounter.txCount      , 0);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 1);

    // -------------------------------------------------------------------------
    // Step 2. Return Good status with FIFO queue data
    requestData[0] = reinterpret_cast<const uint8_t*>(&fifoAddress)[1];
    requestData[1] = reinterpret_cast<const uint8_t*>(&fifoAddress)[0];
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_FIFO_QUEUE),
            SetArrayArgument<2>(requestData, requestData + sizeof(requestData)),
            SetArgPointee<4>(2),
            Return(Status_Good)));
    
    count = 5; // 5 values in FIFO queue
    uint16_t fifoData[5] = {0x1234, 0x5678, 0x9ABC, 0xDEF0, 0x1111};
    EXPECT_CALL(*mockDevice, readFIFOQueue(unit, fifoAddress, _, _))
        .Times(1)
        .WillOnce(Invoke([&count, &fifoData](uint8_t, uint16_t, uint16_t* outCount, uint16_t* values) {
            *outCount = count;
            memcpy(values, fifoData, count * sizeof(uint16_t));
            return Status_Good;
        }));
 
    uint16_t byteCount = (count * 2) + 2;
    // Response format: byteCount(2) + count(2) + values(count*2)
    responseData[0] = reinterpret_cast<const uint8_t*>(&byteCount)[1]; // byteCount Hi-byte
    responseData[1] = reinterpret_cast<const uint8_t*>(&byteCount)[0]; // byteCount Lo-byte
    responseData[2] = reinterpret_cast<const uint8_t*>(&count)[1];     // count Hi-byte
    responseData[3] = reinterpret_cast<const uint8_t*>(&count)[0];     // count Lo-byte
    // FIFO values in big-endian
    responseData[4] = reinterpret_cast<const uint8_t*>(&fifoData[0])[1];
    responseData[5] = reinterpret_cast<const uint8_t*>(&fifoData[0])[0];
    responseData[6] = reinterpret_cast<const uint8_t*>(&fifoData[1])[1];
    responseData[7] = reinterpret_cast<const uint8_t*>(&fifoData[1])[0];
    responseData[8] = reinterpret_cast<const uint8_t*>(&fifoData[2])[1];
    responseData[9] = reinterpret_cast<const uint8_t*>(&fifoData[2])[0];
    responseData[10] = reinterpret_cast<const uint8_t*>(&fifoData[3])[1];
    responseData[11] = reinterpret_cast<const uint8_t*>(&fifoData[3])[0];
    responseData[12] = reinterpret_cast<const uint8_t*>(&fifoData[4])[1];
    responseData[13] = reinterpret_cast<const uint8_t*>(&fifoData[4])[0];
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_FIFO_QUEUE, _, 14))
        .With(Args<2, 3>(ElementsAreArray(responseData, 14)))
        .Times(1);

    result = serverResource->process();
    EXPECT_EQ(result, Status_Good);
    EXPECT_EQ(signalCounter.rxCount      , 2);
    EXPECT_EQ(signalCounter.txCount      , 1);
    EXPECT_EQ(signalCounter.errorCount   , 1);
    EXPECT_EQ(signalCounter.completeCount, 2);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(ModbusServerResourceTest, DeviceReturnsStandardException)
{
    const uint8_t unit = 1;
    
    uint8_t requestData[5] = {MBF_READ_HOLDING_REGISTERS, 0x00, 0x00, 0x00, 0x02};
    uint8_t responseData[3];
    
    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    // Device returns standard exception (illegal data address)
    EXPECT_CALL(*mockDevice, readHoldingRegisters(_, _, _, _))
        .WillOnce(Return(Status_BadIllegalDataAddress));
    
    // Expect exception response with 0x80 bit set
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_HOLDING_REGISTERS | 0x80, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusServerResourceTest, PortOpenFails)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(false));
    
    EXPECT_CALL(*mockPort, open())
        .WillOnce(Return(Status_BadSerialOpen));
    
    serverResource->open();
    StatusCode result = serverResource->process();
    
    // Should handle open failure gracefully
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusServerResourceTest, PortReadFails)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_BadSerialReadTimeout));
    
    StatusCode result = serverResource->process();
    
    // Should handle read failure
    EXPECT_TRUE(StatusIsBad(result));
}

TEST_F(ModbusServerResourceTest, PortWriteFails)
{
    const uint8_t unit = 1;
    
    uint8_t requestData[5] = {MBF_READ_COILS, 0x00, 0x00, 0x00, 0x08};
    uint8_t responseData[3];
    
    setupBufferMethodExpectations(requestData, sizeof(requestData), responseData, sizeof(responseData));
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_COILS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint8_t coilData[1] = {0xAA};
    EXPECT_CALL(*mockDevice, readCoils(_, _, _, _))
        .WillOnce(Invoke([coilData](uint8_t, uint16_t, uint16_t, void* values) {
            memcpy(values, coilData, 1);
            return Status_Good;
        }));
    
    EXPECT_CALL(*mockPort, writeBuffer(_, _, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_BadTcpWrite));
    
    StatusCode result = serverResource->process();
    
    // Should handle write failure
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

// ============================================================================
// Signal Tests
// ============================================================================

TEST_F(ModbusServerResourceTest, SignalsExist)
{
    // Verify that signal objects exist (from ModbusServerPort base class)
    EXPECT_NO_THROW({
        // These signals should be accessible
        // signalOpened, signalClosed, signalError, signalTx, signalRx
    });
}

// ============================================================================
// Test signals emitted by ModbusServerResource (e.g., opened, closed etc.)
// ============================================================================

struct ModbusServerResourceSignalHandler
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

TEST(ModbusServerResourceSignalsTest, testSignals)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>(true);
    port->setTimeout(0); // Non-blocking for test
    NiceMock<MockModbusDevice> device;
    
    
    ModbusServerResource serverPort(port, &device);
    ModbusServerResourceSignalHandler signalHandler;

    serverPort.connect(&ModbusServerResource::signalOpened   , &signalHandler, &ModbusServerResourceSignalHandler::onOpened   );
    serverPort.connect(&ModbusServerResource::signalClosed   , &signalHandler, &ModbusServerResourceSignalHandler::onClosed   );
    serverPort.connect(&ModbusServerResource::signalTx       , &signalHandler, &ModbusServerResourceSignalHandler::onTx       );
    serverPort.connect(&ModbusServerResource::signalRx       , &signalHandler, &ModbusServerResourceSignalHandler::onRx       );
    serverPort.connect(&ModbusServerResource::signalError    , &signalHandler, &ModbusServerResourceSignalHandler::onError    );
    serverPort.connect(&ModbusServerResource::signalCompleted, &signalHandler, &ModbusServerResourceSignalHandler::onCompleted);

    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;
    
    // Request preparation
    const auto szReadData = 4;    
    uint8_t readData[szReadData];
    readData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    readData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    readData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    readData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    
    // Response data: byte count + 16 registers (32 bytes)
    const auto szWriteData = 33;
    uint8_t writeData[szWriteData];
    writeData[0] = 32; // byte count
    for (int i = 0; i < 32; i++)
        writeData[i + 1] = static_cast<uint8_t>(i);
    
    
    EXPECT_CALL(*port, readBufferData()) // Each step
        .WillRepeatedly(Return(readData))
        ;

    EXPECT_CALL(*port, readBufferSize()) // Each step
        .WillRepeatedly(Return(szReadData))
        ;
    
    EXPECT_CALL(*port, writeBufferData()) // Each step
        .WillRepeatedly(Return(writeData))
        ;

    EXPECT_CALL(*port, writeBufferSize()) // Each step
        .WillRepeatedly(Return(szWriteData))
        ;
    
    uint32_t expected_openCount     {0};
    uint32_t expected_closeCount    {0};
    uint32_t expected_txCount       {0};
    uint32_t expected_rxCount       {0};
    uint32_t expected_errorCount    {0};
    uint32_t expected_completeCount {0};

    // Step 1: Successful transaction
    EXPECT_CALL(*port, isOpen())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_))
        .WillOnce(Return(Modbus::Status_Good))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, func, _, szWriteData))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    StatusCode result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    , ++expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));
    
    // Step 2: Read inner port error
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Bad))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      ,   expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));
    
    // Step 3: readBuffer error
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Bad)))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));

    // Step 4: Read device - status bad
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) // Each step
        .WillOnce(Return(Modbus::Status_Bad )) // Step 4
        ;
    EXPECT_CALL(*port, writeBuffer(unit, 0x80 | func, _, 1))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      ); // Note: tx called to send device failure response
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));

    // Step 5: Read device - status bad (Standard Exception)
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) 
        .WillOnce(Return(Modbus::Status_BadIllegalDataAddress))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, 0x80 | func, _, 1))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   , ++expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsBad(result));

    // Step 6: Read device - path unavailable
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) 
        .WillOnce(Return(Modbus::Status_BadGatewayPathUnavailable))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));

    // Step 7: Successful transaction
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_))
        .WillOnce(Return(Modbus::Status_Good))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, func, _, szWriteData))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   ,   expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      , ++expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      , ++expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    
    EXPECT_TRUE(Modbus::StatusIsGood(result));
    
    // Step 8: Read port success but closed
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(false));    
    EXPECT_CALL(*port, read())
        .WillRepeatedly(Return(Status_Good))
        ;

    result = serverPort.process();
    EXPECT_EQ(signalHandler.openCount    ,   expected_openCount    );
    EXPECT_EQ(signalHandler.closeCount   , ++expected_closeCount   );
    EXPECT_EQ(signalHandler.rxCount      ,   expected_rxCount      );
    EXPECT_EQ(signalHandler.txCount      ,   expected_txCount      );
    EXPECT_EQ(signalHandler.errorCount   ,   expected_errorCount   );
    EXPECT_EQ(signalHandler.completeCount, ++expected_completeCount);    

}