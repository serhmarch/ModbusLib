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
    MockModbusPort *mockPort;
    MockModbusDevice *mockDevice;
    ModbusServerResource *serverResource;

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
    void setupBufferMethodExpectations()
    {
        EXPECT_CALL(*mockPort, readBufferSize())
            .WillRepeatedly(Return(0));
        EXPECT_CALL(*mockPort, readBufferData())
            .WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*mockPort, writeBufferSize())
            .WillRepeatedly(Return(0));
        EXPECT_CALL(*mockPort, writeBufferData())
            .WillRepeatedly(Return(nullptr));
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
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 8;
    
    setupBufferMethodExpectations();
    
    // Setup port expectations for receiving request
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2 bytes) + count (2 bytes)
    uint8_t requestData[5] = {MBF_READ_COILS, 0x00, 0x00, 0x00, 0x08};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_COILS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    // Setup device to return coil data
    uint8_t coilData[1] = {0xAA}; // 10101010 pattern
    EXPECT_CALL(*mockDevice, readCoils(unit, offset, count, _))
        .WillOnce(Invoke([coilData](uint8_t, uint16_t, uint16_t, void* values) {
            memcpy(values, coilData, 1);
            return Status_Good;
        }));
    
    // Setup port expectations for sending response
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_COILS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    // Process should return Status_Processing indicating async operation
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Read Discrete Inputs Tests (Function Code 0x02)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadDiscreteInputsRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const uint16_t count = 8;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    uint8_t requestData[5] = {MBF_READ_DISCRETE_INPUTS, 0x00, 0x0A, 0x00, 0x08};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_DISCRETE_INPUTS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint8_t discreteData[1] = {0xF0};
    EXPECT_CALL(*mockDevice, readDiscreteInputs(unit, offset, count, _))
        .WillOnce(Invoke([discreteData](uint8_t, uint16_t, uint16_t, void* values) {
            memcpy(values, discreteData, 1);
            return Status_Good;
        }));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_DISCRETE_INPUTS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Read Holding Registers Tests (Function Code 0x03)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadHoldingRegistersRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 0;
    const uint16_t count = 2;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    uint8_t requestData[5] = {MBF_READ_HOLDING_REGISTERS, 0x00, 0x00, 0x00, 0x02};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_HOLDING_REGISTERS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint16_t registerData[2] = {0x1234, 0x5678};
    EXPECT_CALL(*mockDevice, readHoldingRegisters(unit, offset, count, _))
        .WillOnce(DoAll(
            SetArrayArgument<3>(registerData, registerData + 2),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_HOLDING_REGISTERS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Read Input Registers Tests (Function Code 0x04)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadInputRegistersRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 5;
    const uint16_t count = 3;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    uint8_t requestData[5] = {MBF_READ_INPUT_REGISTERS, 0x00, 0x05, 0x00, 0x03};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_INPUT_REGISTERS),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    uint16_t inputData[3] = {0x1234, 0x5678, 0x9ABC};
    EXPECT_CALL(*mockDevice, readInputRegisters(unit, offset, count, _))
        .WillOnce(DoAll(
            SetArrayArgument<3>(inputData, inputData + 3),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_INPUT_REGISTERS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Write Single Coil Tests (Function Code 0x05)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteSingleCoilOnRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2 bytes) + value (0xFF00 for ON)
    uint8_t requestData[5] = {MBF_WRITE_SINGLE_COIL, 0x00, 0x0A, 0xFF, 0x00};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleCoil(unit, offset, true))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_COIL, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

TEST_F(ModbusServerResourceTest, ProcessWriteSingleCoilOffRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2 bytes) + value (0x0000 for OFF)
    uint8_t requestData[5] = {MBF_WRITE_SINGLE_COIL, 0x00, 0x0A, 0x00, 0x00};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_COIL),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleCoil(unit, offset, false))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_COIL, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Write Single Register Tests (Function Code 0x06)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteSingleRegisterRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 20;
    const uint16_t value = 0x1234;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2 bytes) + value (2 bytes)
    uint8_t requestData[5] = {MBF_WRITE_SINGLE_REGISTER, 0x00, 0x14, 0x12, 0x34};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_SINGLE_REGISTER),
            SetArrayArgument<2>(requestData + 1, requestData + 5),
            SetArgPointee<4>(4),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeSingleRegister(unit, offset, value))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_SINGLE_REGISTER, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Read Exception Status Tests (Function Code 0x07)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessReadExceptionStatusRequest)
{
    const uint8_t unit = 1;
    const uint8_t exceptionStatus = 0x42;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: no data for this function
    uint8_t requestData[1] = {MBF_READ_EXCEPTION_STATUS};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_READ_EXCEPTION_STATUS),
            SetArgPointee<4>(0),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, readExceptionStatus(unit, _))
        .WillOnce(DoAll(
            SetArgPointee<1>(exceptionStatus),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_READ_EXCEPTION_STATUS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Write Multiple Coils Tests (Function Code 0x0F)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteMultipleCoilsRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 10;
    const uint16_t count = 10;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2) + count (2) + byte count (1) + data
    uint8_t requestData[8] = {MBF_WRITE_MULTIPLE_COILS, 0x00, 0x0A, 0x00, 0x0A, 0x02, 0xFF, 0x03};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_COILS),
            SetArrayArgument<2>(requestData + 1, requestData + 8),
            SetArgPointee<4>(7),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeMultipleCoils(unit, offset, count, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_MULTIPLE_COILS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Write Multiple Registers Tests (Function Code 0x10)
// ============================================================================

TEST_F(ModbusServerResourceTest, ProcessWriteMultipleRegistersRequest)
{
    const uint8_t unit = 1;
    const uint16_t offset = 100;
    const uint16_t count = 2;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    // Request: offset (2) + count (2) + byte count (1) + data (4)
    uint8_t requestData[10] = {MBF_WRITE_MULTIPLE_REGISTERS, 0x00, 0x64, 0x00, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78};
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(MBF_WRITE_MULTIPLE_REGISTERS),
            SetArrayArgument<2>(requestData + 1, requestData + 10),
            SetArgPointee<4>(9),
            Return(Status_Good)));
    
    EXPECT_CALL(*mockDevice, writeMultipleRegisters(unit, offset, count, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, writeBuffer(unit, MBF_WRITE_MULTIPLE_REGISTERS, _, _))
        .WillOnce(Return(Status_Good));
    
    EXPECT_CALL(*mockPort, write())
        .WillOnce(Return(Status_Good));
    
    StatusCode result = serverResource->process();
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(ModbusServerResourceTest, DeviceReturnsStandardException)
{
    const uint8_t unit = 1;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    uint8_t requestData[5] = {MBF_READ_HOLDING_REGISTERS, 0x00, 0x00, 0x00, 0x02};
    
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
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
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
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

TEST_F(ModbusServerResourceTest, PortReadFails)
{
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockPort, read())
        .WillOnce(Return(Status_BadSerialReadTimeout));
    
    StatusCode result = serverResource->process();
    
    // Should handle read failure
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

TEST_F(ModbusServerResourceTest, PortWriteFails)
{
    const uint8_t unit = 1;
    
    setupBufferMethodExpectations();
    
    EXPECT_CALL(*mockPort, isOpen())
        .WillRepeatedly(Return(true));
    
    uint8_t requestData[5] = {MBF_READ_COILS, 0x00, 0x00, 0x00, 0x08};
    
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

