include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(democlient democlient.cpp)
target_link_libraries(democlient PRIVATE modbus)

add_executable(mbclient mbclient.c)
set_target_properties(mbclient PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(mbclient PRIVATE modbus)
