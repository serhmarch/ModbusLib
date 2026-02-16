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
    MOCK_METHOD(Modbus::StatusCode, diagnostics               , (uint8_t unit, uint16_t subfunc, uint8_t insize, const void *indata, uint8_t *outsize, void *outdata), (override));
    MOCK_METHOD(Modbus::StatusCode, getCommEventCounter       , (uint8_t unit, uint16_t *status, uint16_t *count), (override));
    MOCK_METHOD(Modbus::StatusCode, getCommEventLog           , (uint8_t unit, uint16_t *status, uint16_t *eventCount, uint16_t *messageCount, uint8_t *eventBuffSize, uint8_t *eventBuff));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleCoils        , (uint8_t unit, uint16_t offset, uint16_t count, const void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleRegisters    , (uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, reportServerID            , (uint8_t unit, uint8_t *count, uint8_t *data), (override));
    MOCK_METHOD(Modbus::StatusCode, maskWriteRegister         , (uint8_t unit, uint16_t offset, uint16_t andMask, uint16_t orMask), (override));
    MOCK_METHOD(Modbus::StatusCode, readWriteMultipleRegisters, (uint8_t unit, uint16_t readOffset, uint16_t readCount, uint16_t *readValues, uint16_t writeOffset, uint16_t writeCount, const uint16_t *writeValues), (override));
    MOCK_METHOD(Modbus::StatusCode, readFIFOQueue             , (uint8_t unit, uint16_t fifoadr, uint16_t *count, uint16_t *values), (override));
};

#endif // MOCKMODBUSDEVICE_H