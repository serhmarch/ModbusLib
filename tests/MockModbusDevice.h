#ifndef MOCKMODBUSDEVICE_H
#define MOCKMODBUSDEVICE_H

#include <gmock/gmock.h>
#include <Modbus.h>

class MockModbusDevice : public ModbusInterface
{
public:
    MockModbusDevice() {}

public:
    MOCK_METHOD(Modbus::StatusCode, readCoils             , (uint8_t unit, uint16_t offset, uint16_t count, void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readDiscreteInputs    , (uint8_t unit, uint16_t offset, uint16_t count, void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readHoldingRegisters  , (uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, readInputRegisters    , (uint8_t unit, uint16_t offset, uint16_t count, uint16_t *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeSingleCoil       , (uint8_t unit, uint16_t offset, bool value), (override));
    MOCK_METHOD(Modbus::StatusCode, writeSingleRegister   , (uint8_t unit, uint16_t offset, uint16_t value), (override));
    MOCK_METHOD(Modbus::StatusCode, readExceptionStatus   , (uint8_t unit, uint8_t *status), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleCoils    , (uint8_t unit, uint16_t offset, uint16_t count, const void *values), (override));
    MOCK_METHOD(Modbus::StatusCode, writeMultipleRegisters, (uint8_t unit, uint16_t offset, uint16_t count, const uint16_t *values), (override));
};

#endif // MOCKMODBUSDEVICE_H