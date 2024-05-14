include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(client client.cpp)
target_link_libraries(client PRIVATE modbus)
