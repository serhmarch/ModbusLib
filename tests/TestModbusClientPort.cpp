#include <gtest/gtest.h>

#include <ModbusClientPort.h>

#include "MockModbusPort.h"

using namespace testing;
using namespace Modbus;

typedef Modbus::StatusCode (ModbusClientPort::*ReadMethodPtr)(uint8_t, uint16_t, uint16_t, void*);
typedef Modbus::StatusCode (ModbusClientPort::*ReadMethodRegsPtr)(uint8_t, uint16_t, uint16_t, uint16_t*);

void testAlgorithmRead(ModbusClientPort *cp, ReadMethodPtr method,
                       uint8_t unit, uint8_t func, uint16_t offset, uint16_t count, uint8_t *outbuff, uint16_t szoutbuff)
{
    MockModbusPort &port = *static_cast<MockModbusPort*>(cp->port());
    //EXPECT_CALL(port, isOpen())
    //    .Times(1)
    //    .WillOnce(Return(true));

    EXPECT_CALL(port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    StatusCode s;
    uint8_t buff[255];
    s = (cp->*method)(unit, offset, count, buff);
    EXPECT_EQ(s, Status_Good);
}

void testAlgorithmWriteSingleCoil(ModbusClientPort *cp,
                                  uint8_t unit, uint16_t offset, bool value, uint8_t *outbuff, uint16_t szoutbuff)
{
    MockModbusPort *port = static_cast<MockModbusPort*>(cp->port());
    const uint8_t func = MBF_WRITE_SINGLE_COIL;
    //EXPECT_CALL(*port, isOpen())
    //    .Times(1)
    //    .WillOnce(Return(true));

    EXPECT_CALL(*port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    StatusCode s;
    s = cp->writeSingleCoil(unit, offset, value);
    EXPECT_EQ(s, Status_Good);
}

void testAlgorithmWriteSingleRegister(ModbusClientPort *cp,
                                  uint8_t unit, uint16_t offset, uint16_t value, uint8_t *outbuff, uint16_t szoutbuff)
{
    MockModbusPort *port = static_cast<MockModbusPort*>(cp->port());
    const uint8_t func = MBF_WRITE_SINGLE_REGISTER;
    //EXPECT_CALL(*port, isOpen())
    //    .Times(1)
    //    .WillOnce(Return(true));

    EXPECT_CALL(*port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    StatusCode s;
    s = cp->writeSingleRegister(unit, offset, value);
    EXPECT_EQ(s, Status_Good);
}

void testAlgorithmReadExceptionStatus(ModbusClientPort *cp,
                       uint8_t unit, uint8_t *outbuff, uint16_t szoutbuff)
{
    MockModbusPort *port = static_cast<MockModbusPort*>(cp->port());
    const uint8_t func = MBF_READ_EXCEPTION_STATUS;
    //EXPECT_CALL(*port, isOpen())
    //    .Times(1)
    //    .WillOnce(Return(true));

    EXPECT_CALL(*port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    StatusCode s;
    uint8_t buff[1];
    s = cp->readExceptionStatus(unit, buff);
    EXPECT_EQ(s, Status_Good);
}

typedef Modbus::StatusCode (ModbusClientPort::*WriteMethodPtr)(uint8_t, uint16_t, uint16_t, const void*);
typedef Modbus::StatusCode (ModbusClientPort::*WriteMethodRegsPtr)(uint8_t, uint16_t, uint16_t, const uint16_t*);

void testAlgorithmWrite(ModbusClientPort *cp, WriteMethodPtr method,
                       uint8_t unit, uint8_t func, uint16_t offset, uint16_t count, uint8_t *outbuff, uint16_t szoutbuff)
{
    MockModbusPort *port = static_cast<MockModbusPort*>(cp->port());
    //EXPECT_CALL(*port, isOpen())
    //    .Times(1)
    //    .WillOnce(Return(true));

    EXPECT_CALL(*port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    StatusCode s;
    uint8_t buff[255];
    s = (cp->*method)(unit, offset, count, buff);
    EXPECT_EQ(s, Status_Good);
}

TEST(ModbusClientPort, testAlgorithmBlocking)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    ModbusClientPort cp(port);
    const uint8_t  unit   = 1;
    const uint16_t offset = 0;
    const uint16_t count  = 16;

    //EXPECT_CALL(*port, open())
    //    .Times(1)
    //    .WillOnce(Return(Status_Good));
    uint8_t buff01[1+count/MB_BYTE_SZ_BITES];
    memset(&buff01[1], 0, sizeof(buff01)-1);
    buff01[0] = count/MB_BYTE_SZ_BITES;
    testAlgorithmRead(&cp, &ModbusClientPort::readCoils, unit, MBF_READ_COILS, offset, count, buff01, sizeof(buff01));

    uint8_t buff02[1+count/MB_BYTE_SZ_BITES];
    memset(&buff02[1], 0, sizeof(buff02)-1);
    buff02[0] = count/MB_BYTE_SZ_BITES;
    testAlgorithmRead(&cp, &ModbusClientPort::readDiscreteInputs, unit, MBF_READ_DISCRETE_INPUTS, offset, count, buff02, sizeof(buff02));

    uint8_t buff03[1+count*MB_REGE_SZ_BYTES];
    memset(&buff03[1], 0, sizeof(buff03)-1);
    buff03[0] = count*MB_REGE_SZ_BYTES;
    ReadMethodRegsPtr ptr03 = &ModbusClientPort::readHoldingRegisters;
    testAlgorithmRead(&cp, reinterpret_cast<ReadMethodPtr>(ptr03), unit, MBF_READ_HOLDING_REGISTERS, offset, count, buff03, sizeof(buff03));

    uint8_t buff04[1+count*MB_REGE_SZ_BYTES];
    memset(&buff04[1], 0, sizeof(buff04)-1);
    buff04[0] = count*MB_REGE_SZ_BYTES;
    ReadMethodRegsPtr ptr04 = &ModbusClientPort::readInputRegisters;
    testAlgorithmRead(&cp, reinterpret_cast<ReadMethodPtr>(ptr04), unit, MBF_READ_INPUT_REGISTERS, offset, count, buff04, sizeof(buff04));

    uint8_t buff05[4];
    memset(&buff05[0], 0, sizeof(buff05));
    bool value05 = false;
    testAlgorithmWriteSingleCoil(&cp, unit, offset, value05, buff05, sizeof(buff05));

    uint8_t buff06[4];
    memset(&buff06[0], 0, sizeof(buff06));
    uint16_t value06 = 0;
    testAlgorithmWriteSingleRegister(&cp, unit, offset, value06, buff06, sizeof(buff06));

    uint8_t buff07[1];
    buff07[0] = 0;
    testAlgorithmReadExceptionStatus(&cp, unit, buff07, sizeof(buff07));

    union { uint8_t buff15[4]; uint16_t buff15reg[2]; };
    buff15reg[0] = offset;
    buff15reg[1] = count;
    testAlgorithmWrite(&cp, &ModbusClientPort::writeMultipleCoils, unit, MBF_WRITE_MULTIPLE_COILS, offset, count, buff15, sizeof(buff15));

    union { uint8_t buff16[4]; uint16_t buff16reg[2]; };
    buff16reg[0] = offset;
    buff16reg[1] = count;
    WriteMethodRegsPtr ptr16 = &ModbusClientPort::writeMultipleRegisters;
    testAlgorithmWrite(&cp, reinterpret_cast<WriteMethodPtr>(ptr16), unit, MBF_WRITE_MULTIPLE_REGISTERS, offset, count, buff16, sizeof(buff16));
}
