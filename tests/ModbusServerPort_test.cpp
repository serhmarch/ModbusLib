#include <gtest/gtest.h>

#include <ModbusServerResource.h>

#include "MockModbusPort.h"
#include "MockModbusDevice.h"

using namespace testing;
using namespace Modbus;

// Helper to build a unit map with specific bits set
static void buildUnitMap(uint8_t *map, const std::initializer_list<uint8_t> &enabledUnits)
{
    memset(map, 0, MB_UNITMAP_SIZE);
    for (uint8_t unit : enabledUnits)
    {
        const uint8_t byteIndex = unit / 8;
        const uint8_t bitIndex  = unit % 8;
        MB_UNITMAP_SET_BIT(map, unit, 1);
    }
}

TEST(ModbusServerPort, testAlgorithm)
{
    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));
    NiceMock<MockModbusDevice> device;

    ModbusServerResource sp(port, &device);

    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;

    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true));

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

TEST(ModbusServerPort_NonVirtual, DeviceGetterSetter)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device1;
    NiceMock<MockModbusDevice> device2;

    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device1);
    EXPECT_EQ(sp.device(), &device1);

    sp.setDevice(&device2);
    EXPECT_EQ(sp.device(), &device2);
}

TEST(ModbusServerPort_NonVirtual, BroadcastEnableBehavior)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device);

    // Default: broadcast enabled
    EXPECT_TRUE(sp.isBroadcastEnabled());
    // Unit 0 should be considered enabled, even with empty unit map
    EXPECT_TRUE(sp.isUnitEnabled(0));

    // Disable broadcast
    sp.setBroadcastEnabled(false);
    EXPECT_FALSE(sp.isBroadcastEnabled());
    // With broadcast disabled and no unit map, all units are enabled by default
    EXPECT_TRUE(sp.isUnitEnabled(0));
}

TEST(ModbusServerPort_NonVirtual, UnitMapSetAndQuery)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device);

    // By default unitMap is nullptr and any unit is enabled
    EXPECT_EQ(sp.unitMap(), nullptr);
    EXPECT_TRUE(sp.isUnitEnabled(0));
    EXPECT_TRUE(sp.isUnitEnabled(1));
    EXPECT_TRUE(sp.isUnitEnabled(10));
    EXPECT_TRUE(sp.isUnitEnabled(200));

    // Provide a specific unit map enabling only units {2, 7, 200}
    uint8_t map[MB_UNITMAP_SIZE];
    buildUnitMap(map, {2, 7, 200});
    sp.setUnitMap(map);
    sp.setBroadcastEnabled(false); // disable broadcast to test map directly

    ASSERT_NE(sp.unitMap(), nullptr);
    EXPECT_FALSE(sp.isUnitEnabled(0)); // broadcast separate, but default enabled; here we explicitly test map without broadcast special-case
    EXPECT_FALSE(sp.isUnitEnabled(1));
    EXPECT_TRUE (sp.isUnitEnabled(2));
    EXPECT_FALSE(sp.isUnitEnabled(3));
    EXPECT_FALSE(sp.isUnitEnabled(5));
    EXPECT_TRUE (sp.isUnitEnabled(7));
    EXPECT_FALSE(sp.isUnitEnabled(8));
    EXPECT_FALSE(sp.isUnitEnabled(199));
    EXPECT_TRUE (sp.isUnitEnabled(200));
    EXPECT_FALSE(sp.isUnitEnabled(201));

    // setUnitEnabled should create map if nullptr and then set bit
    sp.setUnitMap(nullptr);
    ASSERT_EQ(sp.unitMap(), nullptr);
    // if map is nullptr all units enabled
    EXPECT_TRUE(sp.isUnitEnabled(0));
    EXPECT_TRUE(sp.isUnitEnabled(1));
    EXPECT_TRUE(sp.isUnitEnabled(2));
    EXPECT_TRUE(sp.isUnitEnabled(3));

    sp.setUnitEnabled(5, true);
    ASSERT_NE(sp.unitMap(), nullptr);
    EXPECT_TRUE(sp.isUnitEnabled(5));
    // Disable it back
    sp.setUnitEnabled(5, false);
    EXPECT_FALSE(sp.isUnitEnabled(5));
}

TEST(ModbusServerPort_NonVirtual, UnitMapStringSetAndQuery)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device);

    // Initially no unit map string is set
    EXPECT_EQ(sp.unitMap(), nullptr);
    EXPECT_TRUE(sp.unitMapString().empty());

    // Set via string: mix of singles and ranges
    sp.setUnitMapString("2, 7, 10-12, 200");
    sp.setBroadcastEnabled(false); // disable broadcast to test map directly

    ASSERT_NE(sp.unitMap(), nullptr);
    EXPECT_EQ(sp.unitMapString(), "2,7,10-12,200"); // normalized order and ranges

    EXPECT_FALSE(sp.isUnitEnabled(0));
    EXPECT_FALSE(sp.isUnitEnabled(1));
    EXPECT_TRUE (sp.isUnitEnabled(2));
    EXPECT_TRUE (sp.isUnitEnabled(7));
    EXPECT_FALSE(sp.isUnitEnabled(9));
    EXPECT_TRUE (sp.isUnitEnabled(10));
    EXPECT_TRUE (sp.isUnitEnabled(11));
    EXPECT_TRUE (sp.isUnitEnabled(12));
    EXPECT_FALSE(sp.isUnitEnabled(13));
    EXPECT_TRUE (sp.isUnitEnabled(200));
    EXPECT_FALSE(sp.isUnitEnabled(201));

    // Invalid string should not change current map
    sp.setUnitMapString("bad-input");
    EXPECT_EQ(sp.unitMapString(), "2,7,10-12,200");

    // Clear map by empty string -> all units enabled
    sp.setUnitMapString("");
    EXPECT_EQ(sp.unitMap(), nullptr);
    EXPECT_TRUE(sp.unitMapString().empty());
    EXPECT_TRUE(sp.isUnitEnabled(0));
    EXPECT_TRUE(sp.isUnitEnabled(1));
    EXPECT_TRUE(sp.isUnitEnabled(255));
}

TEST(ModbusServerPort_NonVirtual, ContextGetterSetter)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device);

    EXPECT_EQ(sp.context(), nullptr);
    int userData = 42;
    sp.setContext(&userData);
    EXPECT_EQ(sp.context(), &userData);
}

TEST(ModbusServerPort_NonVirtual, IsTcpServerFalseByDefault)
{
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>;
    NiceMock<MockModbusDevice> device;
    EXPECT_CALL(*port, setServerMode(true)).Times(AtLeast(0));

    ModbusServerResource sp(port, &device);
    EXPECT_FALSE(sp.isTcpServer());
}
