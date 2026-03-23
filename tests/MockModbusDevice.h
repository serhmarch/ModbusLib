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

    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnQueryData, (uint8_t unit, const void *indata, uint8_t insize, void *outdata, uint8_t *outsize), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsRestartCommunicationsOption, (uint8_t unit, bool clearEventLog), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnDiagnosticRegister, (uint8_t unit, uint16_t *value), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsChangeAsciiInputDelimiter, (uint8_t unit, char delimiter), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsForceListenOnlyMode, (uint8_t unit), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsClearCountersAndDiagnosticRegister, (uint8_t unit), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusMessageCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusCommunicationErrorCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusExceptionErrorCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerMessageCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerNoResponseCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerNAKCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnServerBusyCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsReturnBusCharacterOverrunCount, (uint8_t unit, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, diagnosticsClearOverrunCounterAndFlag, (uint8_t unit), (override));

    MOCK_METHOD(Modbus::StatusCode, getCommEventCounter       , (uint8_t unit, uint16_t *status, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, getCommEventLog           , (uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, void *eventBuff, uint8_t *eventBuffSize), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleCoils        , (uint8_t unit, uint16_t offset, uint16_t count, const void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleRegisters    , (uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, reportServerID            , (uint8_t unit, void *data, uint8_t *dataSize), (override));
    MOCK_METHOD(Modbus::StatusCode, readFileRecord            , (uint8_t unit, const Modbus::FileRecord *records, uint8_t recordsCount, void *outData, uint8_t *outSize), (override));
    MOCK_METHOD(Modbus::StatusCode, writeFileRecord           , (uint8_t unit, const Modbus::FileRecord *records, uint8_t recordsCount, const void *inData, uint8_t *inSize), (override));
    MOCK_METHOD(Modbus::StatusCode, maskWriteRegister         , (uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask), (override));
    MOCK_METHOD(Modbus::StatusCode, readWriteMultipleRegisters, (uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues), (override));
    MOCK_METHOD(Modbus::StatusCode, readFIFOQueue             , (uint8_t unit, uint16_t fifoadr, uint16_t *values, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, readDeviceIdentification  , (uint8_t unit, uint8_t readDeviceId, uint8_t objectId, void *data, uint8_t *dataSize, uint8_t *numberOfObjects, uint8_t *conformityLevel, bool *moreFollows, uint8_t *nextObjectId), (override));
};

#endif // MOCKMODBUSDEVICE_H