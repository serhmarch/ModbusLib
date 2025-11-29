message("MB: Tests included")

include(${CMAKE_CURRENT_SOURCE_DIR}/gtest_dependency.cmake)

set(MB_TESTS_HEADERS 
    MockModbusPort.h
    MockModbusDevice.h
    )

set(MB_TESTS_SOURCES 
    TestModbus.cpp
    TestModbusAddress.cpp
    TestModbusClientPort.cpp
    TestModbusServerPort.cpp
    TestModbusServerResource.cpp
    TestModbusTcpPort.cpp
    TestModbusTcpServer.cpp
    TestModbusRtuPort.cpp
    TestModbusAscPort.cpp
    main.cpp
    )

# Add Qt-specific tests when Qt support is enabled in the library
if (MB_QT_ENABLED)
    list(APPEND MB_TESTS_SOURCES
        TestModbusQt.cpp
    )
endif()

set(MB_TESTS_EXEC_NAME testmodbus)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_executable(${MB_TESTS_EXEC_NAME} ${GOOGLETEST_SOURCES} ${MB_TESTS_HEADERS} ${MB_TESTS_SOURCES})
target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE modbus)

if (WIN32)
    target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE Ws2_32)
endif()

if (MB_QT_ENABLED)
    # Link Qt::Core if tests include Qt-based helpers
    find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
    target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
endif()
