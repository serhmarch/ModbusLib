cmake_minimum_required(VERSION 3.5) # 2.2 - case insensitive syntax

project(ModbusLib VERSION 0.3.2 LANGUAGES C CXX)

configure_file(${CMAKE_CURRENT_LIST_DIR}/Modbus_config.h.in ${CMAKE_CURRENT_LIST_DIR}/Modbus_config.h)

set(MB_LIBRARY_NAME modbus)

set(HEADERS ${HEADERS} 
    Modbus_config.h         
    ModbusPlatform.h        
    ModbusGlobal.h          
    Modbus.h                
    cModbus.h               
    ModbusObject.h          
    ModbusPort.h            
    ModbusPort_p.h          
    ModbusSerialPort.h      
    ModbusRtuPort.h         
    ModbusAscPort.h         
    ModbusTcpPort.h         
    ModbusClientPort.h      
    ModbusClient_p.h        
    ModbusClient.h          
    ModbusServerPort.h      
    ModbusServerResource.h  
    ModbusTcpServer.h       
    ModbusClientPort_p.h    
    ModbusObject_p.h        
    ModbusSerialPort_p.h    
    ModbusServerPort_p.h    
    ModbusServerResource_p.h
    ModbusTcpPort_p.h       
    ModbusTcpServer_p.h     
    )

set(SOURCES ${SOURCES} 
    Modbus.cpp              
    cModbus.cpp             
    ModbusObject.cpp        
    ModbusPort.cpp          
    ModbusSerialPort.cpp    
    ModbusRtuPort.cpp       
    ModbusAscPort.cpp       
    ModbusTcpPort.cpp       
    ModbusClientPort.cpp    
    ModbusClient.cpp        
    ModbusServerPort.cpp    
    ModbusServerResource.cpp
    ModbusTcpServer.cpp     
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

if (WIN32)
    set(HEADERS ${HEADERS}
        win/Modbus_win.h         
        win/ModbusTCP_win.h         
        win/ModbusSerialPort_p_win.h
        win/ModbusTcpPort_p_win.h   
        win/ModbusTcpServer_p_win.h
        ) 

    set(SOURCES ${SOURCES}
        win/Modbus_win.cpp           
        win/ModbusTcpPort_win.cpp    
        win/ModbusTcpServer_win.cpp  
        win/ModbusSerialPort_win.cpp
        )
else()
    set(HEADERS ${HEADERS}
        unix/ModbusTCP_unix.h         
        unix/ModbusSerialPort_p_unix.h
        unix/ModbusTcpPort_p_unix.h   
        unix/ModbusTcpServer_p_unix.h
        ) 
    set(SOURCES ${SOURCES}
        unix/Modbus_unix.cpp           
        unix/ModbusTcpPort_unix.cpp    
        unix/ModbusTcpServer_unix.cpp  
        unix/ModbusSerialPort_unix.cpp
        )
endif()

set(RESOURCES              
    )     

if (WIN32)
    set(MODBUSLIB_WIN_RESOURCE_FILE win_resource.rc)
    message("${PROJECT_NAME} resource file for Windows: '${MODBUSLIB_WIN_RESOURCE_FILE}'")

    set(MODBUSLIB_WIN_FILE "${MB_LIBRARY_NAME}")
    set(MODBUSLIB_WIN_OUTFILE "${MODBUSLIB_WIN_FILE}.dll")
    configure_file(${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE}.in 
                   ${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE})

    set(RESOURCES ${RESOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/${MODBUSLIB_WIN_RESOURCE_FILE}           
    )     
endif()

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(${MB_LIBRARY_NAME} SHARED ${HEADERS} ${SOURCES} ${RESOURCES})

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

