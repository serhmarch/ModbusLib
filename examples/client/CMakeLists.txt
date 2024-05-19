include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(mbclient mbclient.cpp)
target_link_libraries(mbclient PRIVATE modbus)

add_executable(mbclientc mbclientc.c)
set_target_properties(mbclientc PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(mbclientc PRIVATE modbus)
