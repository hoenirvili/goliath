cmake_minimum_required(VERSION 3.8)

project (
	CFGTrace
	VERSION 0.1
	DESCRIPTION "Control flow graph generator plugin"
	LANGUAGES CXX
)

add_library(CFGTrace SHARED cfgtrace/cfgtrace.cpp)
set_target_properties(CFGTrace PROPERTIES SUFFIX ".aap")
target_include_directories(CFGTrace PUBLIC include/)
target_compile_features(CFGTrace PRIVATE cxx_std_17)
add_subdirectory(cfgtrace)