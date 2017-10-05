cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
file(GLOB jsonrpc_netcat_SRC "*.cpp")
add_executable (jsonrpc_netcat ${jsonrpc_netcat_SRC} )
target_link_libraries (jsonrpc_netcat LINK_PUBLIC  jsonrpc_client imtjson pthread)
 
install(TARGETS jsonrpc_netcat
        DESTINATION "bin"
        )   
        

