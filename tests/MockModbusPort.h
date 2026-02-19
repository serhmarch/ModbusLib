#ifndef MOCKMODBUSPORT_H
#define MOCKMODBUSPORT_H

#include <gmock/gmock.h>
#include <ModbusPort.h>
#include <ModbusPort_p.h>
#include <ModbusFrame_p.h>

class MockModbusFramePrivate : public ModbusFramePrivate
{
public:
    using ModbusFramePrivate::ModbusFramePrivate;

public:
    Modbus::StatusCode writeBuffer(uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff) override
    {
        return Status_Good;
    }

    Modbus::StatusCode readBuffer(uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff) override
    {
        return Status_Good;
    }
};

class MockModbusPortPrivate : public ModbusPortPrivate
{
public:
    using ModbusPortPrivate::ModbusPortPrivate;
};

class MockModbusPort : public ModbusPort
{
public:
    MockModbusPort(bool block = true) : ModbusPort(new MockModbusPortPrivate(new MockModbusFramePrivate(MB_ASC_IO_BUFF_SZ), block))
    {
        setTimeout(1); // default timeout 1 ms for tests
    }

public:
    MOCK_METHOD(Modbus::ProtocolType, type, (), (const, override));
    MOCK_METHOD(Modbus::Handle, handle, (), (const, override));
    MOCK_METHOD(Modbus::StatusCode, open, (), (override));
    MOCK_METHOD(Modbus::StatusCode, close, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, setServerMode, (bool mode), (override));
    MOCK_METHOD(Modbus::StatusCode, writeBuffer, (uint8_t unit, uint8_t func, const uint8_t *buff, uint16_t szInBuff), (override));
    MOCK_METHOD(Modbus::StatusCode, readBuffer, (uint8_t &unit, uint8_t &func, uint8_t *buff, uint16_t maxSzBuff, uint16_t *szOutBuff), (override));
    MOCK_METHOD(Modbus::StatusCode, write, (), (override));
    MOCK_METHOD(Modbus::StatusCode, read, (), (override));
    MOCK_METHOD(const uint8_t*, readBufferData, (), (const, override));
    MOCK_METHOD(uint16_t, readBufferSize, (), (const, override));
    MOCK_METHOD(const uint8_t*, writeBufferData, (), (const, override));
    MOCK_METHOD(uint16_t, writeBufferSize, (), (const, override));
};

#endif // MOCKMODBUSPORT_H