#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusClient.h>
#include <ModbusClientPort.h>
#include <ModbusGlobal.h>

#include "MockModbusPort.h"

using namespace testing;
using namespace Modbus;

class ModbusClientTest : public ::testing::Test
{
protected:
    static constexpr uint8_t kUnit = 7;

    NiceMock<MockModbusPort> *mockPort {nullptr};
    ModbusClientPort *clientPort {nullptr};
    ModbusClient *client {nullptr};

    void SetUp() override
    {
        mockPort = new NiceMock<MockModbusPort>(true);
        EXPECT_CALL(*mockPort, setServerMode(false)).Times(AtLeast(0));
        EXPECT_CALL(*mockPort, isOpen()).WillRepeatedly(Return(true));

        clientPort = new ModbusClientPort(mockPort);
        client = new ModbusClient(kUnit, clientPort);
    }

    void TearDown() override
    {
        delete client;
        delete clientPort;
    }

    template<typename InvokeFn>
    void verifyWrappedCall(uint8_t func,
                           const uint8_t *expectedRequest,
                           uint16_t requestSize,
                           const uint8_t *responseData,
                           uint16_t responseSize,
                           InvokeFn &&invoke)
    {
        EXPECT_CALL(*mockPort, writeBuffer(kUnit, func, _, requestSize))
            .WillOnce(Invoke([&](uint8_t, uint8_t, const void *buff, uint16_t sz) {
                EXPECT_EQ(sz, requestSize);
                if (requestSize)
                    EXPECT_EQ(memcmp(buff, expectedRequest, requestSize), 0);
                return Status_Good;
            }));

        EXPECT_CALL(*mockPort, write())
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPort, read())
            .WillOnce(Return(Status_Good));

        EXPECT_CALL(*mockPort, readBuffer(_, _, _, _, _))
            .WillOnce(DoAll(
                SetArgReferee<0>(kUnit),
                SetArgReferee<1>(func),
                SetArrayArgument<2>(responseData, responseData + responseSize),
                SetArgPointee<4>(responseSize),
                Return(Status_Good)));

        StatusCode result = invoke();
        EXPECT_EQ(result, Status_Good);
    }
};

TEST_F(ModbusClientTest, BasicProperties)
{
    EXPECT_EQ(client->unit(), kUnit);
    client->setUnit(11);
    EXPECT_EQ(client->unit(), 11);
    client->setUnit(kUnit);

    EXPECT_EQ(client->port(), clientPort);
    EXPECT_TRUE(client->isOpen());

    EXPECT_CALL(*mockPort, type())
        .WillOnce(Return(ProtocolType::TCP));
    EXPECT_EQ(client->type(), ProtocolType::TCP);
}

TEST_F(ModbusClientTest, WrappedFunctionsCallPortWithExpectedParams)
{
#ifndef MBF_READ_COILS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08};
        uint8_t resp[] = {0x01, 0xAA};
        uint8_t values[1] = {0};
        verifyWrappedCall(MBF_READ_COILS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readCoils(0, 8, values);
        });
    }
#endif

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08};
        uint8_t resp[] = {0x01, 0x55};
        uint8_t values[1] = {0};
        verifyWrappedCall(MBF_READ_DISCRETE_INPUTS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readDiscreteInputs(0, 8, values);
        });
    }
#endif

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x01};
        uint8_t resp[] = {0x02, 0x12, 0x34};
        uint16_t values[1] = {0};
        verifyWrappedCall(MBF_READ_HOLDING_REGISTERS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readHoldingRegisters(0, 1, values);
        });
    }
#endif

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x01};
        uint8_t resp[] = {0x02, 0x56, 0x78};
        uint16_t values[1] = {0};
        verifyWrappedCall(MBF_READ_INPUT_REGISTERS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readInputRegisters(0, 1, values);
        });
    }
#endif

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    {
        uint8_t req[]  = {0x00, 0x01, 0xFF, 0x00};
        uint8_t resp[] = {0x00, 0x01, 0xFF, 0x00};
        verifyWrappedCall(MBF_WRITE_SINGLE_COIL, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeSingleCoil(1, true);
        });
    }
#endif

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    {
        uint8_t req[]  = {0x00, 0x02, 0x12, 0x34};
        uint8_t resp[] = {0x00, 0x02, 0x12, 0x34};
        verifyWrappedCall(MBF_WRITE_SINGLE_REGISTER, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeSingleRegister(2, 0x1234);
        });
    }
#endif

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    {
        uint8_t resp[] = {0xA5};
        uint8_t value = 0;
        verifyWrappedCall(MBF_READ_EXCEPTION_STATUS, nullptr, 0, resp, sizeof(resp), [&] {
            return client->readExceptionStatus(&value);
        });
    }
#endif

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    {
        uint8_t inData[] = {0x11, 0x22};
        uint8_t req[] = {0x00, 0x00, 0x11, 0x22};
        uint8_t resp[] = {0x00, 0x00, 0x11, 0x22};
        uint8_t outSize = 0;
        uint8_t outData[4] = {0};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnQueryData(sizeof(inData), inData, &outSize, outData);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    {
        uint8_t req[]  = {0x00, 0x01, 0xFF, 0x00};
        uint8_t resp[] = {0x00, 0x01, 0xFF, 0x00};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsRestartCommunicationsOption(true);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    {
        uint8_t req[]  = {0x00, 0x02, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x02, 0x12, 0x34};
        uint16_t value = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnDiagnosticRegister(&value);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    {
        uint8_t req[]  = {0x00, 0x03, static_cast<uint8_t>(':') , 0x00};
        uint8_t resp[] = {0x00, 0x03, static_cast<uint8_t>(':') , 0x00};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsChangeAsciiInputDelimiter(':');
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    {
        uint8_t req[]  = {0x00, 0x04, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x04, 0x00, 0x00};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsForceListenOnlyMode();
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0A, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0A, 0x00, 0x00};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsClearCountersAndDiagnosticRegister();
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0B, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0B, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnBusMessageCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0C, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0C, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnBusCommunicationErrorCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0D, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0D, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnBusExceptionErrorCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0E, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0E, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnServerMessageCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0F, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x0F, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnServerNoResponseCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x10, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x10, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnServerNAKCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x11, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x11, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnServerBusyCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    {
        uint8_t req[]  = {0x00, 0x12, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x12, 0x00, 0x10};
        uint16_t count = 0;
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsReturnBusCharacterOverrunCount(&count);
        });
    }
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    {
        uint8_t req[]  = {0x00, 0x14, 0x00, 0x00};
        uint8_t resp[] = {0x00, 0x14, 0x00, 0x00};
        verifyWrappedCall(MBF_DIAGNOSTICS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->diagnosticsClearOverrunCounterAndFlag();
        });
    }
#endif
#endif // MBF_DIAGNOSTICS_DISABLE

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    {
        uint8_t resp[] = {0xFF, 0xFF, 0x00, 0x21};
        uint16_t status = 0;
        uint16_t eventCount = 0;
        verifyWrappedCall(MBF_GET_COMM_EVENT_COUNTER, nullptr, 0, resp, sizeof(resp), [&] {
            return client->getCommEventCounter(&status, &eventCount);
        });
    }
#endif

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    {
        uint8_t resp[] = {0x06, 0xFF, 0xFF, 0x00, 0x22, 0x00, 0x33};
        uint16_t status = 0;
        uint16_t eventCount = 0;
        uint16_t messageCount = 0;
        uint8_t eventBuffSize = 0;
        uint8_t eventBuff[8] = {0};
        verifyWrappedCall(MBF_GET_COMM_EVENT_LOG, nullptr, 0, resp, sizeof(resp), [&] {
            return client->getCommEventLog(&status, &eventCount, &messageCount, &eventBuffSize, eventBuff);
        });
    }
#endif

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08, 0x01, 0xAA};
        uint8_t resp[] = {0x00, 0x00, 0x00, 0x08};
        uint8_t values[] = {0xAA};
        verifyWrappedCall(MBF_WRITE_MULTIPLE_COILS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeMultipleCoils(0, 8, values);
        });
    }
#endif

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x01, 0x02, 0x12, 0x34};
        uint8_t resp[] = {0x00, 0x00, 0x00, 0x01};
        uint16_t values[] = {0x1234};
        verifyWrappedCall(MBF_WRITE_MULTIPLE_REGISTERS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeMultipleRegisters(0, 1, values);
        });
    }
#endif

#ifndef MBF_REPORT_SERVER_ID_DISABLE
    {
        uint8_t resp[] = {0x02, 0xFF, 0x01};
        uint8_t count = 0;
        uint8_t data[8] = {0};
        verifyWrappedCall(MBF_REPORT_SERVER_ID, nullptr, 0, resp, sizeof(resp), [&] {
            return client->reportServerID(&count, data);
        });
    }
#endif

#ifndef MBF_READ_FILE_RECORD_DISABLE
    {
        FileRecord records[1];
        records[0].fileNumber = 0x0004;
        records[0].recordNumber = 0x0007;
        records[0].recordLength = 0x0002;
        uint8_t req[]  = {0x07, 0x06, 0x00, 0x04, 0x00, 0x07, 0x00, 0x02};
        uint8_t resp[] = {0x07, 0x06, 0x06, 0x12, 0x34, 0x56, 0x78, 0x00};
        uint8_t outSize = 0;
        uint16_t outData[2] = {0, 0};
        verifyWrappedCall(MBF_READ_FILE_RECORD, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readFileRecord(records, 1, outData, &outSize);
        });
    }
#endif

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    {
        FileRecord records[1];
        records[0].fileNumber = 0x0004;
        records[0].recordNumber = 0x0007;
        records[0].recordLength = 0x0002;
        uint16_t inData[2] = {0x1234, 0x5678};
        uint8_t req[]  = {0x04, 0x06, 0x00, 0x04, 0x00};
        uint8_t resp[] = {0x04, 0x06, 0x00, 0x04, 0x00};
        verifyWrappedCall(MBF_WRITE_FILE_RECORD, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeFileRecord(records, 1, inData, nullptr);
        });
    }
#endif

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    {
        uint8_t req[]  = {0x00, 0x32, 0xFF, 0x00, 0x00, 0x12};
        uint8_t resp[] = {0x00, 0x32, 0xFF, 0x00, 0x00, 0x12};
        verifyWrappedCall(MBF_MASK_WRITE_REGISTER, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->maskWriteRegister(50, 0xFF00, 0x0012);
        });
    }
#endif

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x02, 0x56, 0x78};
        uint8_t resp[] = {0x02, 0x12, 0x34};
        uint16_t readValues[1] = {0};
        uint16_t writeValues[1] = {0x5678};
        verifyWrappedCall(MBF_READ_WRITE_MULTIPLE_REGISTERS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readWriteMultipleRegisters(1, 1, readValues, 2, 1, writeValues);
        });
    }
#endif

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    {
        uint8_t req[]  = {0x00, 0x0A};
        uint8_t resp[] = {0x00, 0x04, 0x00, 0x01, 0x12, 0x34};
        uint16_t count = 0;
        uint16_t values[4] = {0};
        verifyWrappedCall(MBF_READ_FIFO_QUEUE, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readFIFOQueue(10, &count, values);
        });
    }
#endif

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
    {
        uint8_t req[]  = {MBF_MEI_READ_DEVICE_ID, MB_MEI_READ_DEVICE_ID_BASIC, 0x00};
        uint8_t resp[] = {MBF_MEI_READ_DEVICE_ID, MB_MEI_READ_DEVICE_ID_BASIC, 0x01, 0x00, 0x00, 0x01, 0x00};
        uint8_t dataSize = 0;
        uint8_t data[8] = {0};
        uint8_t numberOfObjects = 0;
        uint8_t conformityLevel = 0;
        bool moreFollows = false;
        uint8_t nextObjectId = 0;
        verifyWrappedCall(MBF_ENCAPSULATED_INTERFACE_TRANSPORT, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readDeviceIdentification(MB_MEI_READ_DEVICE_ID_BASIC, 0x00, &dataSize, data, &numberOfObjects, &conformityLevel, &moreFollows, &nextObjectId);
        });
    }
#endif
#endif

#ifndef MBF_READ_COILS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08};
        uint8_t resp[] = {0x01, 0x55};
        bool values[8] = {false};
        verifyWrappedCall(MBF_READ_COILS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readCoilsAsBoolArray(0, 8, values);
        });
    }
#endif

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08};
        uint8_t resp[] = {0x01, 0x55};
        bool values[8] = {false};
        verifyWrappedCall(MBF_READ_DISCRETE_INPUTS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->readDiscreteInputsAsBoolArray(0, 8, values);
        });
    }
#endif

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    {
        uint8_t req[]  = {0x00, 0x00, 0x00, 0x08, 0x01, 0x55};
        uint8_t resp[] = {0x00, 0x00, 0x00, 0x08};
        bool values[8] = {true, false, true, false, true, false, true, false};
        verifyWrappedCall(MBF_WRITE_MULTIPLE_COILS, req, sizeof(req), resp, sizeof(resp), [&] {
            return client->writeMultipleCoilsAsBoolArray(0, 8, values);
        });
    }
#endif
}
