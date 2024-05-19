include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(mbserver mbserver.cpp)
target_link_libraries(mbserver PRIVATE modbus)

add_executable(mbserverc mbserverc.c)
set_target_properties(mbserverc PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(mbserverc PRIVATE modbus)
