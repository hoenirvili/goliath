cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project (CFGTrace)


include_directories(include)
add_subdirectory(cfgtrace)

#TODO(hoenir): fix this
target_link_libraries(CFGTrace.app cfgtrace)