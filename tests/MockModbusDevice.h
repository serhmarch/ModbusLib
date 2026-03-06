#ifndef MOCKMODBUSDEVICE_H
#define MOCKMODBUSDEVICE_H

#include <gmock/gmock.h>
#include <Modbus.h>

class MockModbusDevice : public ModbusInterface
{
public:
    MockModbusDevice() {}

public:
    MOCK_METHOD(Modbus::StatusCode, readCoils                 , (uint8_t unit, uint16_t offset, uint16_t count, void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readDiscreteInputs        , (uint8_t unit, uint16_t offset, uint16_t count, void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readHoldingRegisters      , (uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readInputRegisters        , (uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeSingleCoil           , (uint8_t unit, uint16_t offset, bool value), (override));
    MOCK_METHOD(Modbus::StatusCode, writeSingleRegister       , (uint8_t unit, uint16_t offset, uint16_t value), (override));
    MOCK_METHOD(Modbus::StatusCode, readExceptionStatus       , (uint8_t unit, uint8_t *status), (override));
#ifndef MBF_DIAGNOSTICS_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnQueryData, (uint8_t unit, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata), (override));
#endif // MBF_DIAGNOSTICS_RETURN_QUERY_DATA_DISABLE
#ifndef MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsRestartCommunicationsOption, (uint8_t unit, bool clearEventLog), (override));
#endif // MBF_DIAGNOSTICS_RESTART_COMMUNICATIONS_OPTION_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnDiagnosticRegister, (uint8_t unit, uint16_t *value), (override));
#endif // MBF_DIAGNOSTICS_RETURN_DIAGNOSTIC_REGISTER_DISABLE
#ifndef MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsChangeAsciiInputDelimiter, (uint8_t unit, char delimiter), (override));
#endif // MBF_DIAGNOSTICS_CHANGE_ASCII_INPUT_DELIMITER_DISABLE
#ifndef MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsForceListenOnlyMode, (uint8_t unit), (override));
#endif // MBF_DIAGNOSTICS_FORCE_LISTEN_ONLY_MODE_DISABLE
#ifndef MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsClearCountersAndDiagnosticRegister, (uint8_t unit), (override));
#endif // MBF_DIAGNOSTICS_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusMessageCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_BUS_MESSAGE_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusCommunicationErrorCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_BUS_COMMUNICATION_ERROR_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusExceptionErrorCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_BUS_EXCEPTION_ERROR_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerMessageCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_MESSAGE_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerNoResponseCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NO_RESPONSE_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerNAKCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_NAK_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerBusyCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_BUSY_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusCharacterOverrunCount, (uint8_t unit, uint16_t *count), (override));
#endif // MBF_DIAGNOSTICS_RETURN_SERVER_CHARACTER_OVERRUN_COUNT_DISABLE
#ifndef MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
    MOCK_METHOD(Modbus::StatusCode, diagnosticsClearOverrunCounterAndFlag, (uint8_t unit), (override));
#endif // MBF_DIAGNOSTICS_CLEAR_OVERRUN_COUNTER_AND_FLAG_DISABLE
#endif // MBF_DIAGNOSTICS_DISABLE
    //MOCK_METHOD(Modbus::StatusCode, diagnostics               , (uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata), (override));
    MOCK_METHOD(Modbus::StatusCode, getCommEventCounter       , (uint8_t unit, uint16_t *status, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, getCommEventLog           , (uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleCoils        , (uint8_t unit, uint16_t offset, uint16_t count, const void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleRegisters    , (uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, reportServerID            , (uint8_t unit, uint8_t *count, uint8_t *data), (override));
    MOCK_METHOD(Modbus::StatusCode, readFileRecord            , (uint8_t unit, uint8_t recordsCount, const Modbus::FileRecord *records, uint8_t *outSize, void *outData), (override));
    MOCK_METHOD(Modbus::StatusCode, writeFileRecord           , (uint8_t unit, uint8_t recordsCount, const Modbus::FileRecord *records, uint8_t inSize, const void *inData), (override));
    MOCK_METHOD(Modbus::StatusCode, maskWriteRegister         , (uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask), (override));
    MOCK_METHOD(Modbus::StatusCode, readWriteMultipleRegisters, (uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues), (override));
    MOCK_METHOD(Modbus::StatusCode, readFIFOQueue             , (uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values), (override));
};

#endif // MOCKMODBUSDEVICE_H