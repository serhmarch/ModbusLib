cmake_minimum_required(VERSION 3.13) # 2.2 - case insensitive syntax
                                     # 3.13 included policy CMP0077

project(ModbusLib VERSION 0.5.0 LANGUAGES C CXX)

set(MB_LIBRARY_NAME modbus CACHE INTERNAL "Name of the modbus library")

option(BUILD_SHARED_LIBS "Enable building of shared libraries (dll/so)" ON)

option(MB_CLIENT_DISABLE "Disable client part of ModbusLib" OFF)
option(MB_SERVER_DISABLE "Disable server part of ModbusLib" OFF)

option(MB_C_SUPPORT_DISABLE "Disable C language support for ModbusLib" OFF)

option(MBF_READ_COILS_DISABLE                   , OFF)
option(MBF_READ_DISCRETE_INPUTS_DISABLE         , OFF)
option(MBF_READ_HOLDING_REGISTERS_DISABLE       , OFF)
option(MBF_READ_INPUT_REGISTERS_DISABLE         , OFF)
option(MBF_WRITE_SINGLE_COIL_DISABLE            , OFF)
option(MBF_WRITE_SINGLE_REGISTER_DISABLE        , OFF)
option(MBF_READ_EXCEPTION_STATUS_DISABLE        , OFF)
option(MBF_DIAGNOSTICS_DISABLE                  , OFF)
option(MBF_GET_COMM_EVENT_COUNTER_DISABLE       , OFF)
option(MBF_GET_COMM_EVENT_LOG_DISABLE           , OFF)
option(MBF_WRITE_MULTIPLE_COILS_DISABLE         , OFF)
option(MBF_WRITE_MULTIPLE_REGISTERS_DISABLE     , OFF)
option(MBF_REPORT_SERVER_ID_DISABLE             , OFF)
option(MBF_READ_FILE_RECORD_DISABLE             , OFF)
option(MBF_WRITE_FILE_RECORD_DISABLE            , OFF)
option(MBF_MASK_WRITE_REGISTER_DISABLE          , OFF)
option(MBF_READ_WRITE_MULTIPLE_REGISTERS_DISABLE, OFF)
option(MBF_READ_FIFO_QUEUE_DISABLE              , OFF)

option(MB_ADDRESS_CLASS_DISABLE "Disable Modbus::Address class" OFF)

set(MB_DYNAMIC_LINKING OFF)

if (BUILD_SHARED_LIBS)
    set(MB_DYNAMIC_LINKING ON)
    message("MB: compile dynamic '${MB_LIBRARY_NAME}' library")
else()
    message("MB: compile static '${MB_LIBRARY_NAME}' library")
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/Modbus_config.h.in ${CMAKE_CURRENT_LIST_DIR}/Modbus_config.h)

set(HEADERS ${HEADERS} 
    Modbus_config.h         
    ModbusPlatform.h        
    ModbusGlobal.h          
    Modbus.h                
    ModbusObject.h          
    ModbusPort.h            
    ModbusPort_p.h          
    ModbusRtuPort.h         
    ModbusAscPort.h         
    ModbusTcpPort.h         
    ModbusObject_p.h        
    ModbusSerialPort_p.h    
    ModbusTcpPort_p.h       
    )

set(SOURCES ${SOURCES} 
    Modbus.cpp              
    ModbusObject.cpp        
    ModbusPort.cpp          
    ModbusRtuPort.cpp       
    ModbusAscPort.cpp       
    ModbusTcpPort.cpp       
    )     

if (MB_QT_ENABLED)
    message("MB: QT is enabled!")
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

    set(HEADERS ${HEADERS}
        ModbusQt.h         
        ) 

    set(SOURCES ${SOURCES}
        ModbusQt.cpp
        )
else()
    message("MB: QT is disabled!")
endif()


if (MB_CLIENT_DISABLE)
    message("MB: client is disabled!")
else()
    message("MB: client is enabled!")
    set(HEADERS ${HEADERS}
        ModbusClient.h
        ModbusClient_p.h
        ModbusClientPort.h
        ModbusClientPort_p.h
        ) 

    set(SOURCES ${SOURCES}
        ModbusClient.cpp
        ModbusClientPort.cpp
        )
endif()


if (MB_SERVER_DISABLE)
    message("MB: server is disabled!")
else()
    message("MB: server is enabled!")
    set(HEADERS ${HEADERS}
        ModbusServerResource.h
        ModbusServerResource_p.h
        ModbusServerPort.h
        ModbusServerPort_p.h
        ModbusTcpServer.h
        ModbusTcpServer_p.h
        ) 

    set(SOURCES ${SOURCES}
        ModbusServerResource.cpp
        ModbusServerPort.cpp
        ModbusTcpServer.cpp
        )
endif()


if (MB_C_SUPPORT_DISABLE)
    message("MB: C support is disabled!")
else()
    message("MB: C support is enabled!")
    set(HEADERS ${HEADERS}
        cModbus.h
        ) 

    set(SOURCES ${SOURCES}
        cModbus.cpp
        )
endif()


if (WIN32)
    set(HEADERS ${HEADERS}
        win/Modbus_win.h         
        win/ModbusTCP_win.h         
        win/ModbusSerialPort_p_win.h
        win/ModbusTcpPort_p_win.h   
        ) 

    set(SOURCES ${SOURCES}
        win/Modbus_win.cpp           
        win/ModbusTcpPort_win.cpp    
        win/ModbusSerialPort_win.cpp
        )

    if (NOT MB_SERVER_DISABLE)
        set(HEADERS ${HEADERS}
            win/ModbusTcpServer_p_win.h
            ) 

        set(SOURCES ${SOURCES}
            win/ModbusTcpServer_win.cpp  
            )
    endif()

else()
    set(HEADERS ${HEADERS}
        unix/ModbusTCP_unix.h         
        unix/ModbusSerialPort_p_unix.h
        unix/ModbusTcpPort_p_unix.h   
        ) 
    set(SOURCES ${SOURCES}
        unix/Modbus_unix.cpp           
        unix/ModbusTcpPort_unix.cpp    
        unix/ModbusSerialPort_unix.cpp
        )

    if (NOT MB_SERVER_DISABLE)
        set(HEADERS ${HEADERS}
            unix/ModbusTcpServer_p_unix.h
            ) 

        set(SOURCES ${SOURCES}
            unix/ModbusTcpServer_unix.cpp  
            )
    endif()
endif()

set(RESOURCES              
    )     

if (WIN32)
    set(MODBUSLIB_WIN_RESOURCE_FILE win_resource.rc)
    message("MB: ${PROJECT_NAME} resource file for Windows: '${MODBUSLIB_WIN_RESOURCE_FILE}'")

    set(MODBUSLIB_WIN_FILE "${MB_LIBRARY_NAME}")
    set(MODBUSLIB_WIN_OUTFILE "${MODBUSLIB_WIN_FILE}.dll")
    configure_file(${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE}.in 
                   ${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE})

    set(RESOURCES ${RESOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE}           
    )     
endif()

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Rely type of library into BUILD_SHARED_LIBS option
#add_library(${MB_LIBRARY_NAME} SHARED ${HEADERS} ${SOURCES} ${RESOURCES})
add_library(${MB_LIBRARY_NAME} ${HEADERS} ${SOURCES} ${RESOURCES})

target_compile_definitions(${MB_LIBRARY_NAME} PRIVATE MODBUS_EXPORTS)

set_target_properties(
    ${MB_LIBRARY_NAME}
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    )
        
if (WIN32)
    target_link_libraries(${MB_LIBRARY_NAME} PRIVATE Ws2_32 Winmm setupapi)
endif()

if (MB_QT_ENABLED)
    message("MB: Try to link QT library: Qt5::Core")
    target_link_libraries(${MB_LIBRARY_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
endif()

# --------------------- Install pipeline ---------------------

# Set the default install directory within the build directory under an "install" subdirectory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "install prefix" FORCE)
    message(STATUS "No install directory specified, set it to ${CMAKE_INSTALL_PREFIX} as default")
endif ()

set_target_properties(${MB_LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Specify where header and built files should be copied
install(TARGETS ${MB_LIBRARY_NAME}
    EXPORT ${MB_LIBRARY_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${MB_LIBRARY_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin  # For DLLs on Windows
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${MB_LIBRARY_NAME}
    COMPONENT Runtime
    )

# Specify where headers should be found for installed target
target_include_directories(
    ${MB_LIBRARY_NAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    )   

# Create the config file X-targets.cmake
# which will be referenced in config.cmake.in
# Put it in build directory for now to avoid
# having temporary files in the installation directory
install(
    EXPORT ${MB_LIBRARY_NAME}-targets
    FILE "${MB_LIBRARY_NAME}-targets.cmake"
    NAMESPACE ${MB_LIBRARY_NAME}::
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/${MB_LIBRARY_NAME}"
    )

include(CMakePackageConfigHelpers) # Helper to create basic config files

# Write ${MB_LIBRARY_NAME}ConfigVersion.cmake in build directory
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${MB_LIBRARY_NAME}ConfigVersion.cmake"
    VERSION "${CMAKE_PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
    )

# Write MB_LIBRARY_NAMEConfig.cmake in build directory
# (based on config.cmake.in template for dependencies)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${MB_LIBRARY_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${MB_LIBRARY_NAME}"
    )

# Move config files to install directory
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${MB_LIBRARY_NAME}Config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${MB_LIBRARY_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/${MB_LIBRARY_NAME}"
    )