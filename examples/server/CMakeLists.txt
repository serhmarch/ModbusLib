include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(demoserver demoserver.cpp)
target_link_libraries(demoserver PRIVATE modbus)

add_executable(mbserver mbserver.c)
set_target_properties(mbserver PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(mbserver PRIVATE modbus)
