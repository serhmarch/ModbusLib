#ifndef MOCKMODBUSPORT_H
#define MOCKMODBUSPORT_H

#include <gmock/gmock.h>
#include <ModbusPort.h>
#include <ModbusSerialPort_p.h>

class MockModbusPort : public ModbusPort
{
public:
    MockModbusPort(bool block = true) : ModbusPort(ModbusSerialPortPrivate::create(1000, block)) {}

public:
    MOCK_METHOD(Modbus::ProtocolType, type, (), (const, override));
    MOCK_METHOD(Modbus::StatusCode, writeBuffer, (uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff), (override));
    MOCK_METHOD(Modbus::StatusCode, readBuffer, (uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff), (override));
};

#endif // MOCKMODBUSPORT_H