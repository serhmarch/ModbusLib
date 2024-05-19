
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Modbus.h>

using namespace testing;
using namespace Modbus;

TEST(Modbus, crc16)
{
    EXPECT_EQ(crc16(reinterpret_cast<const uint8_t*>("\xDE\xAD\xBE\xAF"), 4), 0x319A);
    EXPECT_EQ(crc16(reinterpret_cast<const uint8_t*>("\x01\x03\x00\x00\x00\x0A"), 6), 0xCDC5);
}

TEST(Modbus, toString)
{
    EXPECT_EQ(toString(0), "0");
    EXPECT_EQ(toString(1), "1");
    EXPECT_EQ(toString(-1), "-1");
}

TEST(Modbus, createPort)
{
    // TODO
}
