#include <Modbus.h>

#include <string>
#include <gtest/gtest.h>

TEST(TestModbusAddress, AddressFromString)
{
    const Modbus::Address bmAdr0xStandard(Modbus::Memory_0x, 0);
    const Modbus::Address bmAdr1xStandard(Modbus::Memory_1x, 0);
    const Modbus::Address bmAdr3xStandard(Modbus::Memory_3x, 0);
    const Modbus::Address bmAdr4xStandard(Modbus::Memory_4x, 0);

    std::string sadr0xStandard("000001");
    std::string sadr1xStandard("100001");
    std::string sadr3xStandard("300001");
    std::string sadr4xStandard("400001");

    std::string sadr0xIEC61131("%Q0");
    std::string sadr1xIEC61131("%I0");
    std::string sadr3xIEC61131("%IW0");
    std::string sadr4xIEC61131("%MW0");

    std::string sadr0xIEC61131Hex("%Q0000h");
    std::string sadr1xIEC61131Hex("%I0000h");
    std::string sadr3xIEC61131Hex("%IW0000h");
    std::string sadr4xIEC61131Hex("%MW0000h");

    Modbus::Address adr0xStandard = Modbus::Address::fromString(sadr0xStandard);   
    Modbus::Address adr1xStandard = Modbus::Address::fromString(sadr1xStandard);
    Modbus::Address adr3xStandard = Modbus::Address::fromString(sadr3xStandard);
    Modbus::Address adr4xStandard = Modbus::Address::fromString(sadr4xStandard);

    EXPECT_EQ(adr0xStandard, bmAdr0xStandard);
    EXPECT_EQ(adr1xStandard, bmAdr1xStandard);
    EXPECT_EQ(adr3xStandard, bmAdr3xStandard);
    EXPECT_EQ(adr4xStandard, bmAdr4xStandard);

    Modbus::Address adr0xIEC61131 = Modbus::Address::fromString(sadr0xIEC61131);   
    Modbus::Address adr1xIEC61131 = Modbus::Address::fromString(sadr1xIEC61131);
    Modbus::Address adr3xIEC61131 = Modbus::Address::fromString(sadr3xIEC61131);
    Modbus::Address adr4xIEC61131 = Modbus::Address::fromString(sadr4xIEC61131);

    EXPECT_EQ(adr0xIEC61131, bmAdr0xStandard);
    EXPECT_EQ(adr1xIEC61131, bmAdr1xStandard);
    EXPECT_EQ(adr3xIEC61131, bmAdr3xStandard);
    EXPECT_EQ(adr4xIEC61131, bmAdr4xStandard);

    Modbus::Address adr0xIEC61131Hex = Modbus::Address::fromString(sadr0xIEC61131Hex);   
    Modbus::Address adr1xIEC61131Hex = Modbus::Address::fromString(sadr1xIEC61131Hex);
    Modbus::Address adr3xIEC61131Hex = Modbus::Address::fromString(sadr3xIEC61131Hex);
    Modbus::Address adr4xIEC61131Hex = Modbus::Address::fromString(sadr4xIEC61131Hex);

    EXPECT_EQ(adr0xIEC61131Hex, bmAdr0xStandard);
    EXPECT_EQ(adr1xIEC61131Hex, bmAdr1xStandard);
    EXPECT_EQ(adr3xIEC61131Hex, bmAdr3xStandard);
    EXPECT_EQ(adr4xIEC61131Hex, bmAdr4xStandard);
}

TEST(TestModbusAddress, AddressToString)
{
    const Modbus::Address bmAdr0xStandard(Modbus::Memory_0x, 0);
    const Modbus::Address bmAdr1xStandard(Modbus::Memory_1x, 0);
    const Modbus::Address bmAdr3xStandard(Modbus::Memory_3x, 0);
    const Modbus::Address bmAdr4xStandard(Modbus::Memory_4x, 0);

    std::string expected0xStandard("000001");
    std::string expected1xStandard("100001");
    std::string expected3xStandard("300001");
    std::string expected4xStandard("400001");

    EXPECT_EQ(bmAdr0xStandard.toString<std::string>(Modbus::Address::Notation_Modbus), expected0xStandard);
    EXPECT_EQ(bmAdr1xStandard.toString<std::string>(Modbus::Address::Notation_Modbus), expected1xStandard);
    EXPECT_EQ(bmAdr3xStandard.toString<std::string>(Modbus::Address::Notation_Modbus), expected3xStandard);
    EXPECT_EQ(bmAdr4xStandard.toString<std::string>(Modbus::Address::Notation_Modbus), expected4xStandard);

    std::string expected0xIEC61131("%Q0");
    std::string expected1xIEC61131("%I0");
    std::string expected3xIEC61131("%IW0");
    std::string expected4xIEC61131("%MW0");

    EXPECT_EQ(bmAdr0xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131), expected0xIEC61131);
    EXPECT_EQ(bmAdr1xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131), expected1xIEC61131);
    EXPECT_EQ(bmAdr3xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131), expected3xIEC61131);
    EXPECT_EQ(bmAdr4xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131), expected4xIEC61131);

    std::string expected0xIEC61131Hex("%Q0000h");
    std::string expected1xIEC61131Hex("%I0000h");
    std::string expected3xIEC61131Hex("%IW0000h");
    std::string expected4xIEC61131Hex("%MW0000h");

    EXPECT_EQ(bmAdr0xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131Hex), expected0xIEC61131Hex);
    EXPECT_EQ(bmAdr1xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131Hex), expected1xIEC61131Hex);
    EXPECT_EQ(bmAdr3xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131Hex), expected3xIEC61131Hex);
    EXPECT_EQ(bmAdr4xStandard.toString<std::string>(Modbus::Address::Notation_IEC61131Hex), expected4xIEC61131Hex);
}