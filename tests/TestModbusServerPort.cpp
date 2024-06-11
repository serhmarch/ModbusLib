#include <gtest/gtest.h>

#include <ModbusServerResource.h>

#include "MockModbusPort.h"
#include "MockModbusDevice.h"

using namespace testing;
using namespace Modbus;

TEST(ModbusServerPort, testAlgorithm)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;

    ModbusServerResource sp(port, &device);

    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;

    EXPECT_CALL(*port, isOpen())
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(*port, read())
        .Times(1)
        .WillOnce(Return(Status_Good));

    uint8_t outbuff[4];
    uint16_t szoutbuff = sizeof(outbuff);
    outbuff[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    outbuff[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    outbuff[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    outbuff[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    EXPECT_CALL(*port, readBuffer(_,_,_,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(unit),
                        SetArgReferee<1>(func),
                        SetArrayArgument<2>(outbuff, outbuff + szoutbuff),
                        SetArgPointee<4>(szoutbuff),
                        Return(Status_Good)));

    uint16_t values[count];
    memset(values, 0 , sizeof(values));
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_))
        .Times(1)
        .WillOnce(DoAll(SetArrayArgument<3>(values, values + count),
                        Return(Status_Good)));

    EXPECT_CALL(*port, writeBuffer(unit,func,_,_))
        .Times(1)
        .WillOnce(Return(Status_Good));

    EXPECT_CALL(*port, write())
        .Times(1)
        .WillOnce(Return(Status_Good));

    sp.process();
}
