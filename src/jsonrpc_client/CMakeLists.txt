cmake_minimum_required(VERSION 2.8) 
add_compile_options(-std=c++11 -fPIC)
file(GLOB jsonrpc_client_SRC "*.cpp")
file(GLOB jsonrpc_client_HDR "*.h" "*.tcc")
add_library (jsonrpc_client ${jsonrpc_client_SRC})

install(TARGETS jsonrpc_client
        DESTINATION "lib"
        ) 
INSTALL(FILES ${jsonrpc_client_HDR}
        DESTINATION "include/jsonrpc_client" 
        )    