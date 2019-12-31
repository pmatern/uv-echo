cmake_minimum_required(VERSION 3.14)
project(test_connector C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(libuv REQUIRED)

add_executable(test_connector main.c)

target_include_directories(test_connector PRIVATE ${LIBUV_INCLUDE_DIRS})
#target_link_libraries(test_connector PRIVATE ${LIBUV_LIBRARIES})
target_link_libraries(test_connector PRIVATE /usr/local/lib/libuv.a)