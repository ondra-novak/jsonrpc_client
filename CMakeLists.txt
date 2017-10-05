cmake_minimum_required(VERSION 2.8)
project (jsonrpc_tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

include_directories(BEFORE src/imtjson/src src)
add_compile_options(-std=c++11)

add_subdirectory (src/imtjson/src/imtjson EXCLUDE_FROM_ALL)
add_subdirectory (src/jsonrpc_client)
add_subdirectory (src/jsonrpc_netcat)


