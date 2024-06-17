message("MB: Tests included")

include(${CMAKE_CURRENT_SOURCE_DIR}/gtest_dependency.cmake)

set(MB_TESTS_HEADERS 
    MockModbusPort.h
    MockModbusDevice.h
    )

set(MB_TESTS_SOURCES 
    TestModbus.cpp
    TestModbusClientPort.cpp
    TestModbusServerPort.cpp
    main.cpp
    )

set(MB_TESTS_EXEC_NAME tests)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")

message("MB: MB_TESTS_EXEC_NAME: ${MB_TESTS_EXEC_NAME}")
message("MB: GOOGLETEST_SOURCES: ${GOOGLETEST_SOURCES}")
message("MB: MB_TESTS_HEADERS: ${MB_TESTS_HEADERS}")
message("MB: MB_TESTS_SOURCES: ${MB_TESTS_SOURCES}")

add_executable(${MB_TESTS_EXEC_NAME} ${GOOGLETEST_SOURCES} ${MB_TESTS_HEADERS} ${MB_TESTS_SOURCES})
target_link_libraries(${MB_TESTS_EXEC_NAME} PRIVATE modbus)
