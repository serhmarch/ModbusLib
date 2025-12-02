message(STATUS "MB: Tests included")

include(${CMAKE_CURRENT_SOURCE_DIR}/gtest_dependency.cmake)

set(MB_TESTS_HEADERS 
    MockModbusPort.h
    MockModbusDevice.h
    )

set(MB_TESTS_SOURCES 
    Modbus_test.cpp
    ModbusAddress_test.cpp
    ModbusClientPort_test.cpp
    ModbusServerPort_test.cpp
    ModbusServerResource_test.cpp
    ModbusTcpPort_test.cpp
    ModbusTcpServer_test.cpp
    ModbusRtuPort_test.cpp
    ModbusAscPort_test.cpp
    main.cpp
    )

# Add Qt-specific tests when Qt support is enabled in the library
if (MB_QT_ENABLED)
    list(APPEND MB_TESTS_SOURCES
        ModbusQt_test.cpp
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

# --- CTest integration ---
include(GoogleTest)

# Register tests by scanning sources; then apply PATH for Windows+Qt at run time
gtest_add_tests(
    TARGET ${MB_TESTS_EXEC_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TEST_LIST MB_DISCOVERED_TESTS
)

if (WIN32 AND MB_QT_ENABLED)
    # Ensure QtCore and modbus DLL dirs are on PATH when running individual tests
    set(_MB_ENV "PATH=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>$<SEMICOLON>$<TARGET_FILE_DIR:modbus>$<SEMICOLON>$ENV{PATH}")
    set_tests_properties(${MB_DISCOVERED_TESTS} PROPERTIES ENVIRONMENT "${_MB_ENV}")
endif()

# Use discovery at test time to avoid running the executable during build.
# On Windows with Qt enabled, ensure PATH contains Qt and modbus DLL folders
# for both discovery (list tests) and execution phases.
#if (MB_QT_ENABLED AND WIN32)
#    message(STATUS "MB-TESTS: Configuring tests with Qt support on Windows for '${MB_TESTS_EXEC_NAME}'")
#    gtest_discover_tests(${MB_TESTS_EXEC_NAME}
#        DISCOVERY_MODE PRE_TEST
#        #PROPERTIES ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY};$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>;$ENV{PATH}"
#        PROPERTIES ENVIRONMENT "PATH=C:\\Qt\\5.15.2\\msvc2019_64\\bin\\;$ENV{PATH}"
#        # WORKING_DIRECTORY can be set if needed
#        # WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#    )
#    #add_test(NAME ${MB_TESTS_EXEC_NAME}-run COMMAND ${MB_TESTS_EXEC_NAME})
#else()
#    gtest_discover_tests(${MB_TESTS_EXEC_NAME}
#        DISCOVERY_MODE PRE_TEST
#    )
#endif()
