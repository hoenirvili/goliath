cmake_minimum_required(VERSION 3.12)
project (CFGTrace VERSION 0.1 DESCRIPTION "Control flow graph generator plugin" LANGUAGES CXX)
add_library(cfgtrace SHARED cfgtrace/cfgtrace.cpp)
target_include_directories(cfgtrace PUBLIC include/)
target_include_directories(cfgtrace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cfgtrace PRIVATE cxx_std_17)
target_compile_definitions(cfgtrace PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(cfgtrace
	PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
		SUFFIX ".aap"
)
add_subdirectory(cfgtrace)

#target_compile_definitions(CFGTrace PRIVATE)
#set(CMAKE_SYSTEM_VERSION 10.0.16299.0 CACHE INTERNAL "")
#set(CMAKE_MFC_FLAG 1)
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")