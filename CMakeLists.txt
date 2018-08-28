cmake_minimum_required(VERSION 3.8)
project (
	CFGTrace 
	VERSION 0.1 
	DESCRIPTION "Control flow graph generator plugin" 
	LANGUAGES CXX
	HOMEPAGE_URL "https://github.com/hoenirvili/CFGTrace"
)

add_library(cfgtrace SHARED cfgtrace/cfgtrace.cpp include/cfgtrace.h)
target_include_directories(cfgtrace PUBLIC include/)
target_include_directories(cfgtrace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cfgtrace PRIVATE cxx_std_17)
target_compile_definitions(cfgtrace PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(cfgtrace
	PROPERTIES
		SUFFIX ".aap"
		OUTPUT_NAME "CFGTrace"
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		LINK_FLAGS "/MANIFEST:NO"
)

add_subdirectory(cfgtrace)
add_subdirectory(demo)