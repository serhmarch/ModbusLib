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
    TestModbusRtuPort.cpp
    TestModbusAscPort.cpp
    main.cpp
    )

set(MB_TESTS_EXEC_NAME testmodbus)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_executable(${MB_TESTS_EXEC_NAME} ${GOOGLETEST_SOURCES} ${MB_TESTS_HEADERS} ${MB_TESTS_SOURCES})
target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE modbus)

if (WIN32)
    target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE Ws2_32)
endif()
