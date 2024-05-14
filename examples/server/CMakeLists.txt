include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(server server.cpp)
target_link_libraries(server PRIVATE modbus)
