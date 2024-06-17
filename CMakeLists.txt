cmake_minimum_required(VERSION 3.5) # 2.2 - case insensitive syntax

project(ModbusLib VERSION 0.1.0 LANGUAGES C CXX)

option(MB_QT_ENABLED "Build Qt additinal info" OFF)
option(MB_EXAMPLES_ENABLED "Build examples" ON)
option(MB_TESTS_ENABLED "Enable unit tests" OFF)

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (NOT WIN32)
#set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)
#set(CMAKE_INSTALL_RPATH "\${ORIGIN}")
endif()

configure_file(Modbus_config.h.in ${PROJECT_SOURCE_DIR}/src/Modbus_config.h)
add_subdirectory(src)

if (MB_EXAMPLES_ENABLED)
    add_subdirectory(examples/client)
    add_subdirectory(examples/server)
endif()

if (MB_TESTS_ENABLED)
    add_subdirectory(tests)
endif()