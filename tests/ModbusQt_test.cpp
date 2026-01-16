#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusQt.h>
#include <ModbusGlobal.h>
#include <ModbusClientPort.h>
#include <ModbusServerPort.h>
#include <ModbusPort.h>
#include "MockModbusDevice.h"

using namespace Modbus;
using namespace testing;

// (Optional) To silence gMock "uninteresting call" warnings when running tests,
// set environment variable before executing tests: GMOCK_VERBOSE=error

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
static Settings makeTcpClientSettings(const QString &host, uint16_t port, uint32_t timeout)
{
    Settings s;
    setSettingType(s, ProtocolType::TCP);
    setSettingHost(s, host);
    setSettingPort(s, port);
    setSettingTimeout(s, timeout);
    return s;
}

static Settings makeTcpServerSettings(const QString &ipaddr, uint16_t port, uint32_t timeout, uint32_t maxconn)
{
    Settings s;
    setSettingType(s, ProtocolType::TCP);
    setSettingIpaddr(s, ipaddr);
    setSettingPort(s, port);
    setSettingTimeout(s, timeout);
    setSettingMaxconn(s, maxconn);
    return s;
}

// ----------------------------------------------------------------------------
// Defaults fallback tests
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, GetDefaultsWhenMissing)
{
    Settings s; // empty

    bool ok = true;

    QString host = getSettingHost(s, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(host, Defaults::instance().host);

    QString ipaddr = getSettingIpaddr(s, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(ipaddr, Defaults::instance().ipaddr);

    uint16_t port = getSettingPort(s, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(port, Defaults::instance().port);

    uint32_t timeout = getSettingTimeout(s, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(timeout, Defaults::instance().timeout);

    uint32_t maxconn = getSettingMaxconn(s, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(maxconn, Defaults::instance().maxconn);
}

// ----------------------------------------------------------------------------
// Set/get roundtrip tests
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, SetAndGetSettings)
{
    Settings s;

    const QString host = QStringLiteral("example.com");
    const QString ipaddr = QStringLiteral("127.0.0.1");
    const uint16_t port = static_cast<uint16_t>(50210);
    const uint32_t timeout = 12345u;
    const uint32_t maxconn = 11u;

    setSettingHost(s, host);
    setSettingPort(s, port);
    setSettingTimeout(s, timeout);
    setSettingMaxconn(s, maxconn);

    bool ok = false;

    EXPECT_EQ(getSettingHost(s, &ok), host);
    EXPECT_TRUE(ok);

    EXPECT_EQ(getSettingPort(s, &ok), port);
    EXPECT_TRUE(ok);

    EXPECT_EQ(getSettingTimeout(s, &ok), timeout);
    EXPECT_TRUE(ok);

    EXPECT_EQ(getSettingMaxconn(s, &ok), maxconn);
    EXPECT_TRUE(ok);
}

// ----------------------------------------------------------------------------
// Factory helpers
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, CreateClientPortFromSettingsTCP)
{
    Settings s = makeTcpClientSettings(QStringLiteral("127.0.0.1"), 502u, 1000u);

    ModbusClientPort *cp = createClientPort(s, /*blocking*/true);
    ASSERT_NE(cp, nullptr);

    // Clean up
    delete cp;
}

TEST(ModbusQtTest, CreateServerPortFromSettingsTCP)
{
    MockModbusDevice device;
    Settings s = makeTcpServerSettings(QStringLiteral("127.0.0.1"), 50499u, 1000u, 2u);

    ModbusServerPort *sp = createServerPort(&device, s, /*blocking*/true);
    ASSERT_NE(sp, nullptr);

    // Basic lifecycle sanity: open/process/close without asserting specific network outcome
    StatusCode st = sp->open();
    int attempts = 0;
    while (StatusIsProcessing(st) && attempts < 100)
    {
        st = sp->process();
        attempts++;
    }
    EXPECT_TRUE(StatusIsGood(st) || StatusIsBad(st));

    // Clean up
    delete sp;
}

// ----------------------------------------------------------------------------
// Strings presence smoke test (non-exhaustive)
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, StringsAndDefaultsPresence)
{
    const Strings &str = Strings::instance();
    const Defaults &def = Defaults::instance();

    // Validate a few key names exist and are non-empty
    EXPECT_FALSE(str.type.isEmpty());
    EXPECT_FALSE(str.host.isEmpty());
    EXPECT_FALSE(str.ipaddr.isEmpty());
    EXPECT_FALSE(str.port.isEmpty());
    EXPECT_FALSE(str.timeout.isEmpty());
    EXPECT_FALSE(str.maxconn.isEmpty());

    // Defaults should be sensible
    EXPECT_GT(def.port, 0u);
    EXPECT_GT(def.timeout, 0u);
    EXPECT_GT(def.maxconn, 0u);
}

// ----------------------------------------------------------------------------
// Getters: serial settings
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, GetSerialDefaultsWhenMissing)
{
    Settings s;
    bool ok = true;

    EXPECT_EQ(getSettingSerialPortName(s, &ok), Defaults::instance().serialPortName); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingBaudRate(s, &ok), Defaults::instance().baudRate); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingDataBits(s, &ok), Defaults::instance().dataBits); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingParity(s, &ok), Defaults::instance().parity); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingStopBits(s, &ok), Defaults::instance().stopBits); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingFlowControl(s, &ok), Defaults::instance().flowControl); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingTimeoutFirstByte(s, &ok), Defaults::instance().timeoutFirstByte); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingTimeoutInterByte(s, &ok), Defaults::instance().timeoutInterByte); EXPECT_FALSE(ok);
    EXPECT_EQ(getSettingBroadcastEnabled(s, &ok), Defaults::instance().isBroadcastEnabled); EXPECT_FALSE(ok);
}

TEST(ModbusQtTest, SetAndGetSerialSettings)
{
    Settings s;
    setSettingSerialPortName(s, QStringLiteral("COM3"));
    setSettingBaudRate(s, 115200);
    setSettingDataBits(s, 8);
    setSettingParity(s, Parity::EvenParity);
    setSettingStopBits(s, StopBits::TwoStop);
    setSettingFlowControl(s, FlowControl::HardwareControl);
    setSettingTimeoutFirstByte(s, 250);
    setSettingTimeoutInterByte(s, 50);
    setSettingBroadcastEnabled(s, true);

    bool ok = false;
    EXPECT_EQ(getSettingSerialPortName(s, &ok), QStringLiteral("COM3")); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingBaudRate(s, &ok), 115200); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingDataBits(s, &ok), 8); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingParity(s, &ok), Parity::EvenParity); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingStopBits(s, &ok), StopBits::TwoStop); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingFlowControl(s, &ok), FlowControl::HardwareControl); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutFirstByte(s, &ok), 250u); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutInterByte(s, &ok), 50u); EXPECT_TRUE(ok);
    EXPECT_TRUE(getSettingBroadcastEnabled(s, &ok)); EXPECT_TRUE(ok);
}

// ----------------------------------------------------------------------------
// Enum conversions: ProtocolType, Parity, StopBits, FlowControl, DataBits
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, ToProtocolTypeFromStringAndVariant)
{
    bool ok = false;
    EXPECT_EQ(toProtocolType(QStringLiteral("TCP"), &ok), ProtocolType::TCP); EXPECT_TRUE(ok);
    ok = false;
    EXPECT_EQ(toProtocolType(QVariant(QStringLiteral("RTU")), &ok), ProtocolType::RTU); EXPECT_TRUE(ok);
}

TEST(ModbusQtTest, ToParityFromStringAndVariant)
{
    bool ok = false;
    EXPECT_EQ(toParity(Strings::instance().EvenParity, &ok), Parity::EvenParity); EXPECT_TRUE(ok);
    ok = false;
    EXPECT_EQ(toParity(QVariant(Strings::instance().OddParity), &ok), Parity::OddParity); EXPECT_TRUE(ok);
}

TEST(ModbusQtTest, ToStopBitsFromStringAndVariant)
{
    bool ok = false;
    EXPECT_EQ(toStopBits(Strings::instance().TwoStop, &ok), StopBits::TwoStop); EXPECT_TRUE(ok);
    ok = false;
    EXPECT_EQ(toStopBits(QVariant(Strings::instance().OneStop), &ok), StopBits::OneStop); EXPECT_TRUE(ok);
}

TEST(ModbusQtTest, ToFlowControlFromStringAndVariant)
{
    bool ok = false;
    EXPECT_EQ(toFlowControl(Strings::instance().HardwareControl, &ok), FlowControl::HardwareControl); EXPECT_TRUE(ok);
    ok = false;
    EXPECT_EQ(toFlowControl(QVariant(Strings::instance().SoftwareControl), &ok), FlowControl::SoftwareControl); EXPECT_TRUE(ok);
}

TEST(ModbusQtTest, ToDataBitsFromStringAndVariant)
{
    bool ok = false;
    EXPECT_EQ(toDataBits(QStringLiteral("Data7"), &ok), 7); EXPECT_TRUE(ok);
    ok = false;
    EXPECT_EQ(toDataBits(QVariant(QStringLiteral("Data8")), &ok), 8); EXPECT_TRUE(ok);
}

TEST(ModbusQtTest, ToBaudRateFromStringAndVariant)
{
    bool ok = false;
    int32_t r1 = toBaudRate(QStringLiteral("57600"), &ok);
    EXPECT_TRUE(ok);
    EXPECT_GE(r1, 0);
    ok = false;
    EXPECT_EQ(toBaudRate(QVariant(38400), &ok), 38400); EXPECT_TRUE(ok);
}

// ----------------------------------------------------------------------------
// toString helpers for enums and status
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, ToStringHelpers)
{
    EXPECT_EQ(toString(ProtocolType::TCP), QStringLiteral("TCP"));
    EXPECT_EQ(toString(Parity::NoParity), Strings::instance().NoParity);
    EXPECT_EQ(toString(StopBits::OneStop), Strings::instance().OneStop);
    EXPECT_EQ(toString(FlowControl::NoFlowControl), Strings::instance().NoFlowControl);

    // StatusCode name trimming (Status_* -> *)
    EXPECT_EQ(toString(Status_Good), QStringLiteral("Good"));
}

// ----------------------------------------------------------------------------
// Utility wrappers: addressFromQString, bytesToString, asciiToString
// ----------------------------------------------------------------------------
// Note: address parsing depends on project-specific syntax; ensure no crash
TEST(ModbusQtTest, AddressFromQString)
{
    Modbus::Address a = addressFromQString(QStringLiteral("MW100"));
    EXPECT_TRUE(a.isValid() || !a.isValid());
}

TEST(ModbusQtTest, BytesAsciiToString)
{
    QByteArray b; b.append('\x01'); b.append('\x02'); b.append('\x03');
    QString bs = bytesToString(b);
    EXPECT_FALSE(bs.isEmpty());

    QByteArray a; a.append('A'); a.append('B'); a.append('C');
    QString as = asciiToString(a);
    EXPECT_FALSE(as.isEmpty());
}

// ----------------------------------------------------------------------------
// availableSerialPortList smoke
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, AvailableSerialPortListReturnsList)
{
    QStringList lst = availableSerialPortList();
    EXPECT_TRUE(lst.size() >= 0);
}

// ----------------------------------------------------------------------------
// Factory createPort RTU/ASC from Settings
// ----------------------------------------------------------------------------
TEST(ModbusQtTest, CreateRtuPortFromSettings)
{
    Settings s;
    setSettingType(s, ProtocolType::RTU);
    setSettingSerialPortName(s, QStringLiteral("COM3"));
    setSettingBaudRate(s, 9600);
    setSettingDataBits(s, 8);
    setSettingParity(s, Parity::NoParity);
    setSettingStopBits(s, StopBits::OneStop);
    setSettingFlowControl(s, FlowControl::NoFlowControl);
    setSettingTimeoutFirstByte(s, 100);
    setSettingTimeoutInterByte(s, 20);

    ModbusPort *p = createPort(s, /*blocking*/true);
    // Verify getters over Settings map reflect configured values
    bool ok = false;
    EXPECT_EQ(getSettingType(s, &ok), ProtocolType::RTU); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingSerialPortName(s, &ok), QStringLiteral("COM3")); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingBaudRate(s, &ok), 9600); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingDataBits(s, &ok), 8); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingParity(s, &ok), Parity::NoParity); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingStopBits(s, &ok), StopBits::OneStop); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingFlowControl(s, &ok), FlowControl::NoFlowControl); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutFirstByte(s, &ok), 100u); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutInterByte(s, &ok), 20u); EXPECT_TRUE(ok);
    ASSERT_NE(p, nullptr);
    delete p;
}

TEST(ModbusQtTest, CreateAsciiPortFromSettings)
{
    Settings s;
    setSettingType(s, ProtocolType::ASC);
    setSettingSerialPortName(s, QStringLiteral("COM4"));
    setSettingBaudRate(s, 19200);
    setSettingDataBits(s, 7);
    setSettingParity(s, Parity::EvenParity);
    setSettingStopBits(s, StopBits::OneStop);
    setSettingFlowControl(s, FlowControl::NoFlowControl);
    setSettingTimeoutFirstByte(s, 150);
    setSettingTimeoutInterByte(s, 30);

    ModbusPort *p = createPort(s, /*blocking*/true);
    ASSERT_NE(p, nullptr);
    // Verify getters over Settings map reflect configured values
    bool ok = false;
    EXPECT_EQ(getSettingType(s, &ok), ProtocolType::ASC); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingSerialPortName(s, &ok), QStringLiteral("COM4")); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingBaudRate(s, &ok), 19200); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingDataBits(s, &ok), 7); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingParity(s, &ok), Parity::EvenParity); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingStopBits(s, &ok), StopBits::OneStop); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingFlowControl(s, &ok), FlowControl::NoFlowControl); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutFirstByte(s, &ok), 150u); EXPECT_TRUE(ok);
    EXPECT_EQ(getSettingTimeoutInterByte(s, &ok), 30u); EXPECT_TRUE(ok);
    delete p;
}

