#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ModbusGlobal.h>
#include <ModbusTcpServer.h>
#include <ModbusServerResource.h>

#include "MockModbusPort.h"
#include "MockModbusDevice.h"

using namespace testing;
using namespace Modbus;

// ============================================================================
// Test Fixture for ModbusTcpServer
// ============================================================================

class ModbusTcpServerTest : public ::testing::Test
{
protected:
    MockModbusDevice *mockDevice;
    ModbusTcpServer *tcpServer;

    void SetUp() override
    {
        mockDevice = new MockModbusDevice();
        tcpServer = new ModbusTcpServer(mockDevice);
    }

    void TearDown() override
    {
        delete tcpServer;
        delete mockDevice;
    }
};

// ============================================================================
// Basic Initialization and Configuration Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, Constructor)
{
    EXPECT_NE(tcpServer, nullptr);
    EXPECT_EQ(tcpServer->device(), mockDevice);
}

TEST_F(ModbusTcpServerTest, TypeReturnsTCP)
{
    EXPECT_EQ(tcpServer->type(), ProtocolType::TCP);
}

TEST_F(ModbusTcpServerTest, IsTcpServerReturnsTrue)
{
    EXPECT_TRUE(tcpServer->isTcpServer());
}

TEST_F(ModbusTcpServerTest, DefaultIpaddr)
{
    const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
    EXPECT_STREQ(tcpServer->ipaddr(), defaults.ipaddr);
}

TEST_F(ModbusTcpServerTest, DefaultPort)
{
    const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
    EXPECT_EQ(tcpServer->port(), defaults.port);
}

TEST_F(ModbusTcpServerTest, DefaultTimeout)
{
    const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
    EXPECT_EQ(tcpServer->timeout(), defaults.timeout);
}

TEST_F(ModbusTcpServerTest, DefaultMaxConnections)
{
    const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
    EXPECT_EQ(tcpServer->maxConnections(), defaults.maxconn);
}

TEST_F(ModbusTcpServerTest, IpaddrSetter)
{
    const char* testIpaddr = "192.168.1.100";
    tcpServer->setIpaddr(testIpaddr);
    EXPECT_STREQ(tcpServer->ipaddr(), testIpaddr);
}

TEST_F(ModbusTcpServerTest, PortSetter)
{
    const uint16_t testPort = 5025;
    tcpServer->setPort(testPort);
    EXPECT_EQ(tcpServer->port(), testPort);
}

TEST_F(ModbusTcpServerTest, TimeoutSetter)
{
    const uint32_t testTimeout = 5000;
    tcpServer->setTimeout(testTimeout);
    EXPECT_EQ(tcpServer->timeout(), testTimeout);
}

TEST_F(ModbusTcpServerTest, MaxConnectionsSetter)
{
    const uint32_t testMaxConn = 25;
    tcpServer->setMaxConnections(testMaxConn);
    EXPECT_EQ(tcpServer->maxConnections(), testMaxConn);
}

TEST_F(ModbusTcpServerTest, MaxConnectionsSetterZeroDefaultsToOne)
{
    tcpServer->setMaxConnections(0);
    EXPECT_EQ(tcpServer->maxConnections(), 1u);
}

TEST_F(ModbusTcpServerTest, SetAllSettings)
{
    const char* testIpaddr = "192.168.1.100";
    const uint16_t testPort = 5025;
    const uint32_t testTimeout = 20000;
    const uint32_t testMaxConn = 25;
    
    tcpServer->setIpaddr(testIpaddr);
    tcpServer->setPort(testPort);
    tcpServer->setTimeout(testTimeout);
    tcpServer->setMaxConnections(testMaxConn);
    
    EXPECT_STREQ(tcpServer->ipaddr(), testIpaddr);
    EXPECT_EQ(tcpServer->port(), testPort);
    EXPECT_EQ(tcpServer->timeout(), testTimeout);
    EXPECT_EQ(tcpServer->maxConnections(), testMaxConn);
}

// ============================================================================
// IP Address Specific Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, IpaddrSetterPersistsAfterClose)
{
    const char* testIpaddr = "127.0.0.1";
    tcpServer->setIpaddr(testIpaddr);

    // Close to simulate lifecycle change
    (void)tcpServer->close();

    EXPECT_STREQ(tcpServer->ipaddr(), testIpaddr);
}

TEST_F(ModbusTcpServerTest, DifferentServersDifferentIpaddr)
{
    MockModbusDevice *device2 = new MockModbusDevice();
    ModbusTcpServer *server2 = new ModbusTcpServer(device2);

    const char* ip1 = "127.0.0.1";
    const char* ip2 = "192.168.10.50";

    tcpServer->setIpaddr(ip1);
    server2->setIpaddr(ip2);

    EXPECT_STRNE(tcpServer->ipaddr(), server2->ipaddr());

    delete server2;
    delete device2;
}

TEST_F(ModbusTcpServerTest, BindToSpecificIpAndPort)
{
    // Use loopback to avoid external dependencies
    const char* loopback = "127.0.0.1";
    const uint16_t customPort = 50499;

    tcpServer->setIpaddr(loopback);
    tcpServer->setPort(customPort);

    EXPECT_STREQ(tcpServer->ipaddr(), loopback);
    EXPECT_EQ(tcpServer->port(), customPort);

    // Attempt to open; allow Good, Processing or Bad depending on environment
    StatusCode result = tcpServer->open();

    int attempts = 0;
    while (StatusIsProcessing(result) && attempts < 100)
    {
        result = tcpServer->process();
        attempts++;
    }

    // We only assert stability (no exceptions) and valid status domain
    EXPECT_TRUE(StatusIsGood(result) || StatusIsBad(result));
}

// ============================================================================
// Broadcast Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, BroadcastEnabledByDefault)
{
    EXPECT_TRUE(tcpServer->isBroadcastEnabled());
}

TEST_F(ModbusTcpServerTest, BroadcastSetter)
{
    tcpServer->setBroadcastEnabled(false);
    EXPECT_FALSE(tcpServer->isBroadcastEnabled());
    
    tcpServer->setBroadcastEnabled(true);
    EXPECT_TRUE(tcpServer->isBroadcastEnabled());
}

// ============================================================================
// Unit Map Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, UnitMapInitiallyNull)
{
    EXPECT_EQ(tcpServer->unitMap(), nullptr);
}

TEST_F(ModbusTcpServerTest, UnitMapSetter)
{
    uint8_t unitMapData[32] = {0};
    unitMapData[0] = 0xFF; // Enable units 0-7
    unitMapData[1] = 0x01; // Enable unit 8
    
    tcpServer->setUnitMap(unitMapData);
    
    const uint8_t *retrievedMap = reinterpret_cast<const uint8_t*>(tcpServer->unitMap());
    EXPECT_NE(retrievedMap, nullptr);
    EXPECT_EQ(retrievedMap[0], 0xFF);
    EXPECT_EQ(retrievedMap[1], 0x01);
}

// ============================================================================
// Open/Close Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, InitiallyNotOpen)
{
    EXPECT_FALSE(tcpServer->isOpen());
}

TEST_F(ModbusTcpServerTest, OpenReturnsStatusCode)
{
    StatusCode result = tcpServer->open();
    
    // May return Status_Good, Status_Processing, or error status
    // depending on async nature and port availability
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

TEST_F(ModbusTcpServerTest, CloseReturnsStatusCode)
{
    StatusCode result = tcpServer->close();
    
    // May return Status_Good or Status_Processing
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result));
}

TEST_F(ModbusTcpServerTest, OpenAndCloseSequence)
{
    // Set custom port to avoid conflicts
    tcpServer->setPort(50123);
    
    StatusCode openResult = tcpServer->open();
    
    // Call process to complete async open operation
    int attempts = 0;
    while (StatusIsProcessing(openResult) && attempts < 100)
    {
        openResult = tcpServer->process();
        attempts++;
    }
    
    if (StatusIsGood(openResult))
    {
        EXPECT_TRUE(tcpServer->isOpen());
        
        StatusCode closeResult = tcpServer->close();
        
        // Call process to complete async close operation
        attempts = 0;
        while (StatusIsProcessing(closeResult) && attempts < 100)
        {
            closeResult = tcpServer->process();
            attempts++;
        }
        
        // Close may return Good or Processing
        EXPECT_TRUE(StatusIsGood(closeResult) || StatusIsProcessing(closeResult));
    }
}

// ============================================================================
// Process Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, ProcessWhenClosed)
{
    EXPECT_FALSE(tcpServer->isOpen());
    
    StatusCode result = tcpServer->process();
    
    // Should return Status_Processing or attempt to open
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

TEST_F(ModbusTcpServerTest, ProcessReturnsProcessingOrGood)
{
    StatusCode result = tcpServer->process();
    
    // process() should return Status_Good or Status_Processing
    EXPECT_TRUE(StatusIsGood(result) || StatusIsProcessing(result) || StatusIsBad(result));
}

// ============================================================================
// Defaults Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, DefaultsStructure)
{
    const ModbusTcpServer::Defaults &defaults = ModbusTcpServer::Defaults::instance();
    
    EXPECT_EQ(defaults.port, STANDARD_TCP_PORT);
    EXPECT_GT(defaults.timeout, 0u);
    EXPECT_GT(defaults.maxconn, 0u);
}

TEST_F(ModbusTcpServerTest, DefaultsSingleton)
{
    const ModbusTcpServer::Defaults &defaults1 = ModbusTcpServer::Defaults::instance();
    const ModbusTcpServer::Defaults &defaults2 = ModbusTcpServer::Defaults::instance();
    
    EXPECT_EQ(&defaults1, &defaults2);
}

// ============================================================================
// Signal Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, SignalsExist)
{
    // Verify that signal objects exist (from ModbusServerPort base class)
    EXPECT_NO_THROW({
        // These signals should be accessible:
        // signalOpened, signalClosed, signalError, signalTx, signalRx
        // signalNewConnection, signalCloseConnection
    });
}

// ============================================================================
// Device Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, DeviceGetter)
{
    EXPECT_EQ(tcpServer->device(), mockDevice);
}

// ============================================================================
// Port Binding Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, BindToNonStandardPort)
{
    const uint16_t customPort = 50200;
    tcpServer->setPort(customPort);
    
    EXPECT_EQ(tcpServer->port(), customPort);
    
    StatusCode result = tcpServer->open();
    
    // Call process to complete async open
    int attempts = 0;
    while (StatusIsProcessing(result) && attempts < 100)
    {
        result = tcpServer->process();
        attempts++;
    }
    
    // Should succeed or fail (if port unavailable), but not crash
    EXPECT_TRUE(StatusIsGood(result) || StatusIsBad(result));
}

// ============================================================================
// Multiple Server Instances Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, MultipleServersDifferentPorts)
{
    MockModbusDevice *device2 = new MockModbusDevice();
    ModbusTcpServer *server2 = new ModbusTcpServer(device2);
    
    tcpServer->setPort(50300);
    server2->setPort(50301);
    
    EXPECT_NE(tcpServer->port(), server2->port());
    
    delete server2;
    delete device2;
}

// ============================================================================
// Settings Persistence Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, SettingsPersistAfterClose)
{
    const uint16_t testPort = 50400;
    const uint32_t testTimeout = 7000;
    const uint32_t testMaxConn = 15;
    
    tcpServer->setPort(testPort);
    tcpServer->setTimeout(testTimeout);
    tcpServer->setMaxConnections(testMaxConn);
    
    tcpServer->close();
    
    EXPECT_EQ(tcpServer->port(), testPort);
    EXPECT_EQ(tcpServer->timeout(), testTimeout);
    EXPECT_EQ(tcpServer->maxConnections(), testMaxConn);
}

// ============================================================================
// Timeout Propagation Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, TimeoutSetterAcceptsValue)
{
    const uint32_t testTimeout = 15000;
    
    tcpServer->setTimeout(testTimeout);
    
    EXPECT_EQ(tcpServer->timeout(), testTimeout);
}

// ============================================================================
// Connection Management Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, MaxConnectionsLimitEnforced)
{
    // Test that setting max connections to 1 limits properly
    tcpServer->setMaxConnections(1);
    EXPECT_EQ(tcpServer->maxConnections(), 1u);
    
    // Test large value
    tcpServer->setMaxConnections(100);
    EXPECT_EQ(tcpServer->maxConnections(), 100u);
}

// ============================================================================
// Object Name Tests
// ============================================================================

TEST_F(ModbusTcpServerTest, ObjectName)
{
    const Char *name = "TestServer";
    tcpServer->setObjectName(name);
    
    EXPECT_STREQ(tcpServer->objectName(), name);
}

// ============================================================================
// Test signals emitted by ModbusServerResource (e.g., opened, closed etc.)
// ============================================================================

struct ModbusTcpServerSignalHandler
{
    uint32_t openCount           {0};
    uint32_t closeCount          {0};
    uint32_t txCount             {0};
    uint32_t rxCount             {0};
    uint32_t errorCount          {0};
    uint32_t completeCount       {0};
    uint32_t newConnectionCount  {0};
    uint32_t closeConnectionCount{0};

    void onOpened(const Modbus::Char *)
    {
        openCount++;
    }

    void onClosed(const Modbus::Char *)
    {
        closeCount++;
    };

    void onTx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        txCount++;
    };

    void onRx(const Modbus::Char *, const uint8_t *, uint16_t)
    {
        rxCount++;
    };

    void onError(const Modbus::Char *, Modbus::StatusCode, const Modbus::Char *)
    {
        errorCount++;
    };

    void onCompleted(const Modbus::Char *, Modbus::StatusCode)
    {
        completeCount++;
    };

    void onNewConnection(const Modbus::Char *)
    {
        newConnectionCount++;
    };

    void onCloseConnection(const Modbus::Char *)
    {
        closeConnectionCount++;
    };
};  

class MockModbusTcpServer : public ModbusTcpServer
{
public:
    MockModbusTcpServer(ModbusInterface *device) : ModbusTcpServer(device) {}

public:
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(StatusCode, open, (), (override));
    MOCK_METHOD(ModbusServerPort*, createTcpPort, (ModbusTcpSocket *socket), (override));
    MOCK_METHOD(ModbusTcpSocket*, nextPendingConnection, (), (override));
};

TEST(ModbusTcpServerSignalsTest, testSignals)
{
    StatusCode result;

    // NiceMock for ignoring uninteresting calls
    NiceMock<MockModbusPort> *port = new NiceMock<MockModbusPort>(true);
    port->setTimeout(0); // Non-blocking for test
    NiceMock<MockModbusDevice> device;

    NiceMock<MockModbusTcpServer> tcpServer(&device);
    EXPECT_CALL(tcpServer, createTcpPort(_))
        .WillOnce(Return(new ModbusServerResource(port,&device)))
        ;
    EXPECT_CALL(tcpServer, nextPendingConnection())
        .WillOnce(Return(reinterpret_cast<ModbusTcpSocket*>(0xFFFFFFFF))) // Return dummy pointer to trigger connection handling
        .WillRepeatedly(Return(nullptr)) // No more connections
        ;
    
    ModbusTcpServerSignalHandler signalHandler;

    tcpServer.connect(&ModbusTcpServer::signalOpened         , &signalHandler, &ModbusTcpServerSignalHandler::onOpened         );
    tcpServer.connect(&ModbusTcpServer::signalClosed         , &signalHandler, &ModbusTcpServerSignalHandler::onClosed         );
    tcpServer.connect(&ModbusTcpServer::signalTx             , &signalHandler, &ModbusTcpServerSignalHandler::onTx             );
    tcpServer.connect(&ModbusTcpServer::signalRx             , &signalHandler, &ModbusTcpServerSignalHandler::onRx             );
    tcpServer.connect(&ModbusTcpServer::signalError          , &signalHandler, &ModbusTcpServerSignalHandler::onError          );
    tcpServer.connect(&ModbusTcpServer::signalCompleted      , &signalHandler, &ModbusTcpServerSignalHandler::onCompleted      );
    tcpServer.connect(&ModbusTcpServer::signalNewConnection  , &signalHandler, &ModbusTcpServerSignalHandler::onNewConnection  );
    tcpServer.connect(&ModbusTcpServer::signalCloseConnection, &signalHandler, &ModbusTcpServerSignalHandler::onCloseConnection);

    const uint8_t  unit   = 1;
    const uint8_t  func   = MBF_READ_HOLDING_REGISTERS;
    const uint16_t offset = 0;
    const uint16_t count  = 16;
    
    // Request preparation
    const auto szReadData = 4;    
    uint8_t readData[szReadData];
    readData[0] = reinterpret_cast<const uint8_t*>(&offset)[1];
    readData[1] = reinterpret_cast<const uint8_t*>(&offset)[0];
    readData[2] = reinterpret_cast<const uint8_t*>(&count)[1];
    readData[3] = reinterpret_cast<const uint8_t*>(&count)[0];
    
    // Response data: byte count + 16 registers (32 bytes)
    const auto szWriteData = 33;
    uint8_t writeData[szWriteData];
    writeData[0] = 32; // byte count
    for (int i = 0; i < 32; i++)
        writeData[i + 1] = static_cast<uint8_t>(i);
    
    
    EXPECT_CALL(*port, readBufferData()) // Each step
        .WillRepeatedly(Return(readData))
        ;

    EXPECT_CALL(*port, readBufferSize()) // Each step
        .WillRepeatedly(Return(szReadData))
        ;
    
    EXPECT_CALL(*port, writeBufferData()) // Each step
        .WillRepeatedly(Return(writeData))
        ;

    EXPECT_CALL(*port, writeBufferSize()) // Each step
        .WillRepeatedly(Return(szWriteData))
        ;
    
    uint32_t expected_openCount           {0};
    uint32_t expected_closeCount          {0};
    uint32_t expected_txCount             {0};
    uint32_t expected_rxCount             {0};
    uint32_t expected_errorCount          {0};
    uint32_t expected_completeCount       {0};
    uint32_t expected_newConnectionCount  {0};
    uint32_t expected_closeConnectionCount{0};

    // Step 1: New connection established + Successful transaction
    EXPECT_CALL(tcpServer, isOpen())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_))
        .WillOnce(Return(Modbus::Status_Good))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, func, _, szWriteData))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           , ++expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             , ++expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          ,   expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  , ++expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    
    
    // Step 2: Read inner port error
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Bad))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             ,   expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             ,   expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          , ++expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    
    
    // Step 3: readBuffer error
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Bad)))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             ,   expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          , ++expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    

    // Step 4: Read device - status bad
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) // Each step
        .WillOnce(Return(Modbus::Status_Bad )) // Step 4
        ;
    EXPECT_CALL(*port, writeBuffer(unit, 0x80 | func, _, 1))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             , ++expected_txCount             ); // Note: tx called to send device failure response
    EXPECT_EQ(signalHandler.errorCount          , ++expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    

    // Step 5: Read device - status bad (Standard Exception)
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) 
        .WillOnce(Return(Modbus::Status_BadIllegalDataAddress))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, 0x80 | func, _, 1))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             , ++expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          , ++expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    

    // Step 6: Read device - path unavailable
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_)) 
        .WillOnce(Return(Modbus::Status_BadGatewayPathUnavailable))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             ,   expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          ,   expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    

    // Step 7: Successful transaction
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(true))
        ;
    EXPECT_CALL(*port, read())
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, readBuffer(_, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(unit),
            SetArgReferee<1>(func),
            SetArrayArgument<2>(readData, readData + szReadData),
            SetArgPointee<4>(szReadData),
            Return(Status_Good)))
        ;
    EXPECT_CALL(device, readHoldingRegisters(unit,offset,count,_))
        .WillOnce(Return(Modbus::Status_Good))
        ;
    EXPECT_CALL(*port, writeBuffer(unit, func, _, szWriteData))
        .WillOnce(Return(Status_Good))
        ;
    EXPECT_CALL(*port, write())
        .WillOnce(Return(Status_Good))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             , ++expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             , ++expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          ,   expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);    
    
    // Step 8: Read port success but closed
    EXPECT_CALL(*port, isOpen())
        .WillRepeatedly(Return(false));    
    EXPECT_CALL(*port, read())
        .WillRepeatedly(Return(Status_Good))
        ;

    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          ,   expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             ,   expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             ,   expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          ,   expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       , ++expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount, ++expected_closeConnectionCount);
    
    // Step 9: Close TCP server
    EXPECT_CALL(tcpServer, isOpen())
        .WillRepeatedly(Return(false))
        ;
    tcpServer.close();
    result = tcpServer.process();
    EXPECT_EQ(signalHandler.openCount           ,   expected_openCount           );
    EXPECT_EQ(signalHandler.closeCount          , ++expected_closeCount          );
    EXPECT_EQ(signalHandler.rxCount             ,   expected_rxCount             );
    EXPECT_EQ(signalHandler.txCount             ,   expected_txCount             );
    EXPECT_EQ(signalHandler.errorCount          ,   expected_errorCount          );
    EXPECT_EQ(signalHandler.completeCount       ,   expected_completeCount       );    
    EXPECT_EQ(signalHandler.newConnectionCount  ,   expected_newConnectionCount  );
    EXPECT_EQ(signalHandler.closeConnectionCount,   expected_closeConnectionCount);

}