#include <gtest/gtest.h>

#include <cModbus.h>
#include <Modbus.h>

namespace {

using namespace Modbus;

constexpr uint8_t kUnit = 7;

enum CallbackId
{
    CbReadCoils = 1,
    CbReadDiscreteInputs,
    CbReadHoldingRegisters,
    CbReadInputRegisters,
    CbWriteSingleCoil,
    CbWriteSingleRegister,
    CbReadExceptionStatus,
    CbDiagnosticsReturnQueryData,
    CbDiagnosticsRestartCommunicationsOption,
    CbDiagnosticsReturnDiagnosticRegister,
    CbDiagnosticsChangeAsciiInputDelimiter,
    CbDiagnosticsForceListenOnlyMode,
    CbDiagnosticsClearCountersAndDiagnosticRegister,
    CbDiagnosticsReturnBusMessageCount,
    CbDiagnosticsReturnBusCommunicationErrorCount,
    CbDiagnosticsReturnBusExceptionErrorCount,
    CbDiagnosticsReturnServerMessageCount,
    CbDiagnosticsReturnServerNoResponseCount,
    CbDiagnosticsReturnServerNAKCount,
    CbDiagnosticsReturnServerBusyCount,
    CbDiagnosticsReturnBusCharacterOverrunCount,
    CbDiagnosticsClearOverrunCounterAndFlag,
    CbGetCommEventCounter,
    CbGetCommEventLog,
    CbWriteMultipleCoils,
    CbWriteMultipleRegisters,
    CbReportServerID,
    CbReadFileRecord,
    CbWriteFileRecord,
    CbMaskWriteRegister,
    CbReadWriteMultipleRegisters,
    CbReadFIFOQueue,
    CbReadDeviceIdentification
};

inline StatusCode cbStatus(int id)
{
    return static_cast<StatusCode>(0x200 + id);
}

struct CallbackCtx
{
    int called[64] {};
};

#ifndef MBF_READ_COILS_DISABLE
StatusCode cbReadCoils(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadCoils];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 11);
    EXPECT_EQ(count, 5);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbReadCoils);
}
#endif

#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
StatusCode cbReadDiscreteInputs(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, void *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadDiscreteInputs];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 12);
    EXPECT_EQ(count, 6);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbReadDiscreteInputs);
}
#endif

#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
StatusCode cbReadHoldingRegisters(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadHoldingRegisters];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 13);
    EXPECT_EQ(count, 2);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbReadHoldingRegisters);
}
#endif

#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
StatusCode cbReadInputRegisters(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadInputRegisters];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 14);
    EXPECT_EQ(count, 2);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbReadInputRegisters);
}
#endif

#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
StatusCode cbWriteSingleCoil(cModbusDevice dev, uint8_t unit, uint16_t offset, bool value)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbWriteSingleCoil];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 15);
    EXPECT_TRUE(value);
    return cbStatus(CbWriteSingleCoil);
}
#endif

#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
StatusCode cbWriteSingleRegister(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t value)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbWriteSingleRegister];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 16);
    EXPECT_EQ(value, 0x1234);
    return cbStatus(CbWriteSingleRegister);
}
#endif

#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
StatusCode cbReadExceptionStatus(cModbusDevice dev, uint8_t unit, uint8_t *status)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadExceptionStatus];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(status, nullptr);
    *status = 0xA5;
    return cbStatus(CbReadExceptionStatus);
}
#endif

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
StatusCode cbDiagnosticsReturnQueryData(cModbusDevice dev, uint8_t unit, const void *indata, uint8_t insize, void *outdata, uint8_t *outsize)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnQueryData];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(indata, nullptr);
    EXPECT_EQ(insize, 2);
    EXPECT_NE(outdata, nullptr);
    EXPECT_NE(outsize, nullptr);
    *outsize = 2;
    return cbStatus(CbDiagnosticsReturnQueryData);
}
#endif

#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
StatusCode cbDiagnosticsRestartCommunicationsOption(cModbusDevice dev, uint8_t unit, bool clearEventLog)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsRestartCommunicationsOption];
    EXPECT_EQ(unit, kUnit);
    EXPECT_TRUE(clearEventLog);
    return cbStatus(CbDiagnosticsRestartCommunicationsOption);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cbDiagnosticsReturnDiagnosticRegister(cModbusDevice dev, uint8_t unit, uint16_t *value)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnDiagnosticRegister];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(value, nullptr);
    *value = 0x55AA;
    return cbStatus(CbDiagnosticsReturnDiagnosticRegister);
}
#endif

#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
StatusCode cbDiagnosticsChangeAsciiInputDelimiter(cModbusDevice dev, uint8_t unit, char delimiter)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsChangeAsciiInputDelimiter];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(delimiter, ':');
    return cbStatus(CbDiagnosticsChangeAsciiInputDelimiter);
}
#endif

#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
StatusCode cbDiagnosticsForceListenOnlyMode(cModbusDevice dev, uint8_t unit)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsForceListenOnlyMode];
    EXPECT_EQ(unit, kUnit);
    return cbStatus(CbDiagnosticsForceListenOnlyMode);
}
#endif

#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
StatusCode cbDiagnosticsClearCountersAndDiagnosticRegister(cModbusDevice dev, uint8_t unit)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsClearCountersAndDiagnosticRegister];
    EXPECT_EQ(unit, kUnit);
    return cbStatus(CbDiagnosticsClearCountersAndDiagnosticRegister);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
StatusCode cbDiagnosticsReturnBusMessageCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnBusMessageCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 1;
    return cbStatus(CbDiagnosticsReturnBusMessageCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
StatusCode cbDiagnosticsReturnBusCommunicationErrorCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnBusCommunicationErrorCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 2;
    return cbStatus(CbDiagnosticsReturnBusCommunicationErrorCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
StatusCode cbDiagnosticsReturnBusExceptionErrorCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnBusExceptionErrorCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 3;
    return cbStatus(CbDiagnosticsReturnBusExceptionErrorCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
StatusCode cbDiagnosticsReturnServerMessageCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnServerMessageCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 4;
    return cbStatus(CbDiagnosticsReturnServerMessageCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
StatusCode cbDiagnosticsReturnServerNoResponseCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnServerNoResponseCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 5;
    return cbStatus(CbDiagnosticsReturnServerNoResponseCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
StatusCode cbDiagnosticsReturnServerNAKCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnServerNAKCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 6;
    return cbStatus(CbDiagnosticsReturnServerNAKCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
StatusCode cbDiagnosticsReturnServerBusyCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnServerBusyCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 7;
    return cbStatus(CbDiagnosticsReturnServerBusyCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
StatusCode cbDiagnosticsReturnBusCharacterOverrunCount(cModbusDevice dev, uint8_t unit, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsReturnBusCharacterOverrunCount];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(count, nullptr);
    *count = 8;
    return cbStatus(CbDiagnosticsReturnBusCharacterOverrunCount);
}
#endif

#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
StatusCode cbDiagnosticsClearOverrunCounterAndFlag(cModbusDevice dev, uint8_t unit)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbDiagnosticsClearOverrunCounterAndFlag];
    EXPECT_EQ(unit, kUnit);
    return cbStatus(CbDiagnosticsClearOverrunCounterAndFlag);
}
#endif
#endif

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
StatusCode cbGetCommEventCounter(cModbusDevice dev, uint8_t unit, uint16_t *status, uint16_t *eventCount)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbGetCommEventCounter];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(status, nullptr);
    EXPECT_NE(eventCount, nullptr);
    *status = 0x1111;
    *eventCount = 0x2222;
    return cbStatus(CbGetCommEventCounter);
}
#endif

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
StatusCode cbGetCommEventLog(cModbusDevice dev, uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, void *eventBuff, uint8_t *eventBuffSize)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbGetCommEventLog];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(status, nullptr);
    EXPECT_NE(eventCount, nullptr);
    EXPECT_NE(messageCount, nullptr);
    EXPECT_NE(eventBuff, nullptr);
    EXPECT_NE(eventBuffSize, nullptr);
    *status = 0x3333;
    *eventCount = 0x4444;
    *messageCount = 0x5555;
    *eventBuffSize = 3;
    return cbStatus(CbGetCommEventLog);
}
#endif

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
StatusCode cbWriteMultipleCoils(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const void *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbWriteMultipleCoils];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 21);
    EXPECT_EQ(count, 3);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbWriteMultipleCoils);
}
#endif

#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cbWriteMultipleRegisters(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbWriteMultipleRegisters];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 22);
    EXPECT_EQ(count, 2);
    EXPECT_NE(values, nullptr);
    return cbStatus(CbWriteMultipleRegisters);
}
#endif

#ifndef MBF_REPORT_SERVER_ID_DISABLE
StatusCode cbReportServerID(cModbusDevice dev, uint8_t unit, void *data, uint8_t *dataSize)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReportServerID];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(dataSize, nullptr);
    *dataSize = 2;
    return cbStatus(CbReportServerID);
}
#endif

#ifndef MBF_READ_FILE_RECORD_DISABLE
StatusCode cbReadFileRecord(cModbusDevice dev, uint8_t unit, const FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadFileRecord];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(records, nullptr);
    EXPECT_EQ(recordsCount, 1);
    EXPECT_NE(outData, nullptr);
    EXPECT_NE(outSize, nullptr);
    *outSize = 4;
    return cbStatus(CbReadFileRecord);
}
#endif

#ifndef MBF_WRITE_FILE_RECORD_DISABLE
StatusCode cbWriteFileRecord(cModbusDevice dev, uint8_t unit, const FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbWriteFileRecord];
    EXPECT_EQ(unit, kUnit);
    EXPECT_NE(records, nullptr);
    EXPECT_EQ(recordsCount, 1);
    EXPECT_NE(inData, nullptr);
    EXPECT_NE(inSize, nullptr);
    *inSize = 4;
    return cbStatus(CbWriteFileRecord);
}
#endif

#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
StatusCode cbMaskWriteRegister(cModbusDevice dev, uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbMaskWriteRegister];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(offset, 23);
    EXPECT_EQ(andMask, 0x0FF0);
    EXPECT_EQ(orMask, 0x000A);
    return cbStatus(CbMaskWriteRegister);
}
#endif

#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
StatusCode cbReadWriteMultipleRegisters(cModbusDevice dev, uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadWriteMultipleRegisters];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(readOffset, 24);
    EXPECT_EQ(readCount, 2);
    EXPECT_NE(readValues, nullptr);
    EXPECT_EQ(writeOffset, 26);
    EXPECT_EQ(writeCount, 2);
    EXPECT_NE(writeValues, nullptr);
    return cbStatus(CbReadWriteMultipleRegisters);
}
#endif

#ifndef MBF_READ_FIFO_QUEUE_DISABLE
StatusCode cbReadFIFOQueue(cModbusDevice dev, uint8_t unit, uint16_t fifoadr, uint16_t *values, uint16_t *count)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadFIFOQueue];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(fifoadr, 27);
    EXPECT_NE(values, nullptr);
    EXPECT_NE(count, nullptr);
    *count = 2;
    return cbStatus(CbReadFIFOQueue);
}
#endif

#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
StatusCode cbReadDeviceIdentification(cModbusDevice dev, uint8_t unit, uint8_t readDeviceId, uint8_t objectId, void *data, uint8_t *dataSize, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId)
{
    auto *ctx = static_cast<CallbackCtx*>(dev);
    ++ctx->called[CbReadDeviceIdentification];
    EXPECT_EQ(unit, kUnit);
    EXPECT_EQ(readDeviceId, 1);
    EXPECT_EQ(objectId, 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(dataSize, nullptr);
    EXPECT_NE(numberOfObjects, nullptr);
    EXPECT_NE(conformityLevel, nullptr);
    EXPECT_NE(moreFollows, nullptr);
    EXPECT_NE(nextObjectId, nullptr);
    *dataSize = 3;
    *numberOfObjects = 1;
    *conformityLevel = 1;
    *moreFollows = false;
    *nextObjectId = 0;
    return cbStatus(CbReadDeviceIdentification);
}
#endif
#endif

TEST(cModbusCTest, CreateDeviceMapsCallbacksToModbusInterface)
{
    CallbackCtx ctx {};
    cModbusFunctions functions {};

#ifndef MBF_READ_COILS_DISABLE
    functions.readCoils = cbReadCoils;
#endif
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    functions.readDiscreteInputs = cbReadDiscreteInputs;
#endif
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    functions.readHoldingRegisters = cbReadHoldingRegisters;
#endif
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    functions.readInputRegisters = cbReadInputRegisters;
#endif
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    functions.writeSingleCoil = cbWriteSingleCoil;
#endif
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    functions.writeSingleRegister = cbWriteSingleRegister;
#endif
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    functions.readExceptionStatus = cbReadExceptionStatus;
#endif
#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    functions.diagnosticsReturnQueryData = cbDiagnosticsReturnQueryData;
#endif
#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    functions.diagnosticsRestartCommunicationsOption = cbDiagnosticsRestartCommunicationsOption;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    functions.diagnosticsReturnDiagnosticRegister = cbDiagnosticsReturnDiagnosticRegister;
#endif
#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    functions.diagnosticsChangeAsciiInputDelimiter = cbDiagnosticsChangeAsciiInputDelimiter;
#endif
#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    functions.diagnosticsForceListenOnlyMode = cbDiagnosticsForceListenOnlyMode;
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    functions.diagnosticsClearCountersAndDiagnosticRegister = cbDiagnosticsClearCountersAndDiagnosticRegister;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    functions.diagnosticsReturnBusMessageCount = cbDiagnosticsReturnBusMessageCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    functions.diagnosticsReturnBusCommunicationErrorCount = cbDiagnosticsReturnBusCommunicationErrorCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    functions.diagnosticsReturnBusExceptionErrorCount = cbDiagnosticsReturnBusExceptionErrorCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    functions.diagnosticsReturnServerMessageCount = cbDiagnosticsReturnServerMessageCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    functions.diagnosticsReturnServerNoResponseCount = cbDiagnosticsReturnServerNoResponseCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    functions.diagnosticsReturnServerNAKCount = cbDiagnosticsReturnServerNAKCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    functions.diagnosticsReturnServerBusyCount = cbDiagnosticsReturnServerBusyCount;
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    functions.diagnosticsReturnBusCharacterOverrunCount = cbDiagnosticsReturnBusCharacterOverrunCount;
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    functions.diagnosticsClearOverrunCounterAndFlag = cbDiagnosticsClearOverrunCounterAndFlag;
#endif
#endif
#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    functions.getCommEventCounter = cbGetCommEventCounter;
#endif
#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    functions.getCommEventLog = cbGetCommEventLog;
#endif
#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    functions.writeMultipleCoils = cbWriteMultipleCoils;
#endif
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    functions.writeMultipleRegisters = cbWriteMultipleRegisters;
#endif
#ifndef MBF_REPORT_SERVER_ID_DISABLE
    functions.reportServerID = cbReportServerID;
#endif
#ifndef MBF_READ_FILE_RECORD_DISABLE
    functions.readFileRecord = cbReadFileRecord;
#endif
#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    functions.writeFileRecord = cbWriteFileRecord;
#endif
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    functions.maskWriteRegister = cbMaskWriteRegister;
#endif
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    functions.readWriteMultipleRegisters = cbReadWriteMultipleRegisters;
#endif
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    functions.readFIFOQueue = cbReadFIFOQueue;
#endif
#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
    functions.readDeviceIdentification = cbReadDeviceIdentification;
#endif
#endif

    cModbusInterface cdev = cCreateModbusDevice(&ctx, &functions);

    ASSERT_NE(cdev, nullptr);
    auto *iface = static_cast<ModbusInterface*>(cdev);

    uint8_t bits[8] {};
    uint16_t regs[8] {};
    uint8_t oneByte = 0;
    uint16_t oneWord = 0;

#ifndef MBF_READ_COILS_DISABLE
    EXPECT_EQ(iface->readCoils(kUnit, 11, 5, bits), cbStatus(CbReadCoils));
    EXPECT_EQ(ctx.called[CbReadCoils], 1);
#endif
#ifndef MBF_READ_DISCRETE_INPUTS_DISABLE
    EXPECT_EQ(iface->readDiscreteInputs(kUnit, 12, 6, bits), cbStatus(CbReadDiscreteInputs));
    EXPECT_EQ(ctx.called[CbReadDiscreteInputs], 1);
#endif
#ifndef MBF_READ_HOLDING_REGISTERS_DISABLE
    EXPECT_EQ(iface->readHoldingRegisters(kUnit, 13, 2, regs), cbStatus(CbReadHoldingRegisters));
    EXPECT_EQ(ctx.called[CbReadHoldingRegisters], 1);
#endif
#ifndef MBF_READ_INPUT_REGISTERS_DISABLE
    EXPECT_EQ(iface->readInputRegisters(kUnit, 14, 2, regs), cbStatus(CbReadInputRegisters));
    EXPECT_EQ(ctx.called[CbReadInputRegisters], 1);
#endif
#ifndef MBF_WRITE_SINGLE_COIL_DISABLE
    EXPECT_EQ(iface->writeSingleCoil(kUnit, 15, true), cbStatus(CbWriteSingleCoil));
    EXPECT_EQ(ctx.called[CbWriteSingleCoil], 1);
#endif
#ifndef MBF_WRITE_SINGLE_REGISTER_DISABLE
    EXPECT_EQ(iface->writeSingleRegister(kUnit, 16, 0x1234), cbStatus(CbWriteSingleRegister));
    EXPECT_EQ(ctx.called[CbWriteSingleRegister], 1);
#endif
#ifndef MBF_READ_EXCEPTION_STATUS_DISABLE
    EXPECT_EQ(iface->readExceptionStatus(kUnit, &oneByte), cbStatus(CbReadExceptionStatus));
    EXPECT_EQ(oneByte, 0xA5);
    EXPECT_EQ(ctx.called[CbReadExceptionStatus], 1);
#endif

#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    {
        uint8_t inData[2] {0x11, 0x22};
        uint8_t outData[8] {};
        uint8_t outSize = 0;
        EXPECT_EQ(iface->diagnosticsReturnQueryData(kUnit, inData, 2, outData, &outSize), cbStatus(CbDiagnosticsReturnQueryData));
        EXPECT_EQ(outSize, 2);
        EXPECT_EQ(ctx.called[CbDiagnosticsReturnQueryData], 1);
    }
#endif
#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    EXPECT_EQ(iface->diagnosticsRestartCommunicationsOption(kUnit, true), cbStatus(CbDiagnosticsRestartCommunicationsOption));
    EXPECT_EQ(ctx.called[CbDiagnosticsRestartCommunicationsOption], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnDiagnosticRegister(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnDiagnosticRegister));
    EXPECT_EQ(oneWord, 0x55AA);
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnDiagnosticRegister], 1);
#endif
#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    EXPECT_EQ(iface->diagnosticsChangeAsciiInputDelimiter(kUnit, ':'), cbStatus(CbDiagnosticsChangeAsciiInputDelimiter));
    EXPECT_EQ(ctx.called[CbDiagnosticsChangeAsciiInputDelimiter], 1);
#endif
#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    EXPECT_EQ(iface->diagnosticsForceListenOnlyMode(kUnit), cbStatus(CbDiagnosticsForceListenOnlyMode));
    EXPECT_EQ(ctx.called[CbDiagnosticsForceListenOnlyMode], 1);
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    EXPECT_EQ(iface->diagnosticsClearCountersAndDiagnosticRegister(kUnit), cbStatus(CbDiagnosticsClearCountersAndDiagnosticRegister));
    EXPECT_EQ(ctx.called[CbDiagnosticsClearCountersAndDiagnosticRegister], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnBusMessageCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnBusMessageCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnBusMessageCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnBusCommunicationErrorCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnBusCommunicationErrorCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnBusCommunicationErrorCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnBusExceptionErrorCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnBusExceptionErrorCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnBusExceptionErrorCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnServerMessageCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnServerMessageCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnServerMessageCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnServerNoResponseCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnServerNoResponseCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnServerNoResponseCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnServerNAKCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnServerNAKCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnServerNAKCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnServerBusyCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnServerBusyCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnServerBusyCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    EXPECT_EQ(iface->diagnosticsReturnBusCharacterOverrunCount(kUnit, &oneWord), cbStatus(CbDiagnosticsReturnBusCharacterOverrunCount));
    EXPECT_EQ(ctx.called[CbDiagnosticsReturnBusCharacterOverrunCount], 1);
#endif
#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    EXPECT_EQ(iface->diagnosticsClearOverrunCounterAndFlag(kUnit), cbStatus(CbDiagnosticsClearOverrunCounterAndFlag));
    EXPECT_EQ(ctx.called[CbDiagnosticsClearOverrunCounterAndFlag], 1);
#endif
#endif

#ifndef MBF_GET_COMM_EVENT_COUNTER_DISABLE
    {
        uint16_t status = 0;
        uint16_t eventCount = 0;
        EXPECT_EQ(iface->getCommEventCounter(kUnit, &status, &eventCount), cbStatus(CbGetCommEventCounter));
        EXPECT_EQ(status, 0x1111);
        EXPECT_EQ(eventCount, 0x2222);
        EXPECT_EQ(ctx.called[CbGetCommEventCounter], 1);
    }
#endif

#ifndef MBF_GET_COMM_EVENT_LOG_DISABLE
    {
        uint16_t status = 0;
        uint16_t eventCount = 0;
        uint16_t messageCount = 0;
        uint8_t eventBuff[8] {};
        uint8_t eventBuffSize = 0;
        EXPECT_EQ(iface->getCommEventLog(kUnit, &status, &eventCount, &messageCount, eventBuff, &eventBuffSize), cbStatus(CbGetCommEventLog));
        EXPECT_EQ(eventBuffSize, 3);
        EXPECT_EQ(ctx.called[CbGetCommEventLog], 1);
    }
#endif

#ifndef MBF_WRITE_MULTIPLE_COILS_DISABLE
    EXPECT_EQ(iface->writeMultipleCoils(kUnit, 21, 3, bits), cbStatus(CbWriteMultipleCoils));
    EXPECT_EQ(ctx.called[CbWriteMultipleCoils], 1);
#endif
#ifndef MBF_WRITE_MULTIPLE_REGISTERS_DISABLE
    EXPECT_EQ(iface->writeMultipleRegisters(kUnit, 22, 2, regs), cbStatus(CbWriteMultipleRegisters));
    EXPECT_EQ(ctx.called[CbWriteMultipleRegisters], 1);
#endif
#ifndef MBF_REPORT_SERVER_ID_DISABLE
    {
        uint8_t data[8] {};
        uint8_t dataSize = 0;
        EXPECT_EQ(iface->reportServerID(kUnit, data, &dataSize), cbStatus(CbReportServerID));
        EXPECT_EQ(dataSize, 2);
        EXPECT_EQ(ctx.called[CbReportServerID], 1);
    }
#endif
#ifndef MBF_READ_FILE_RECORD_DISABLE
    {
        FileRecord rec {1, 2, 2};
        uint8_t outData[16] {};
        uint8_t outSize = 0;
        EXPECT_EQ(iface->readFileRecord(kUnit, &rec, 1, outData, &outSize), cbStatus(CbReadFileRecord));
        EXPECT_EQ(outSize, 4);
        EXPECT_EQ(ctx.called[CbReadFileRecord], 1);
    }
#endif
#ifndef MBF_WRITE_FILE_RECORD_DISABLE
    {
        FileRecord rec {1, 2, 2};
        uint8_t inData[16] {0xAA, 0xBB, 0xCC, 0xDD};
        uint8_t inSize = 0;
        EXPECT_EQ(iface->writeFileRecord(kUnit, &rec, 1, inData, &inSize), cbStatus(CbWriteFileRecord));
        EXPECT_EQ(inSize, 4);
        EXPECT_EQ(ctx.called[CbWriteFileRecord], 1);
    }
#endif
#ifndef MBF_MASK_WRITE_REGISTER_DISABLE
    EXPECT_EQ(iface->maskWriteRegister(kUnit, 23, 0x0FF0, 0x000A), cbStatus(CbMaskWriteRegister));
    EXPECT_EQ(ctx.called[CbMaskWriteRegister], 1);
#endif
#ifndef MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE
    {
        uint16_t readValues[2] {};
        uint16_t writeValues[2] {0x1111, 0x2222};
        EXPECT_EQ(iface->readWriteMultipleRegisters(kUnit, 24, 2, readValues, 26, 2, writeValues), cbStatus(CbReadWriteMultipleRegisters));
        EXPECT_EQ(ctx.called[CbReadWriteMultipleRegisters], 1);
    }
#endif
#ifndef MBF_READ_FIFO_QUEUE_DISABLE
    {
        uint16_t values[8] {};
        uint16_t count = 0;
        EXPECT_EQ(iface->readFIFOQueue(kUnit, 27, values, &count), cbStatus(CbReadFIFOQueue));
        EXPECT_EQ(count, 2);
        EXPECT_EQ(ctx.called[CbReadFIFOQueue], 1);
    }
#endif
#ifndef MBF_ENCAPSULATED_INTERFACE_TRANSPORT_DISABLE
#ifndef MBF_MEI_READ_DEVICE_IDENTIFICATION_DISABLE
    {
        uint8_t data[16] {};
        uint8_t dataSize = 0;
        uint8_t numberOfObjects = 0;
        uint8_t conformityLevel = 0;
        bool moreFollows = true;
        uint8_t nextObjectId = 1;
        EXPECT_EQ(iface->readDeviceIdentification(kUnit, 1, 0, data, &dataSize, &numberOfObjects, &conformityLevel, &moreFollows, &nextObjectId), cbStatus(CbReadDeviceIdentification));
        EXPECT_EQ(dataSize, 3);
        EXPECT_EQ(numberOfObjects, 1);
        EXPECT_FALSE(moreFollows);
        EXPECT_EQ(nextObjectId, 0);
        EXPECT_EQ(ctx.called[CbReadDeviceIdentification], 1);
    }
#endif
#endif

    cDeleteModbusDevice(cdev);
}

} // namespace
