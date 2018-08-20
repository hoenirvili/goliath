add_library(cfgtracelib 
	cfgtrace.cpp
	instruction.cpp
	node.cpp
	control_flow_graph.cpp
)
#TODO(hoenir): test this
target_include_directories(cfgtracelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(CFGTrace PRIVATE cxx_std_17)

add_subdirectory(command)
add_subdirectory(engine)
add_subdirectory(error)
add_subdirectory(format)
add_subdirectory(log)
add_subdirectory(random)