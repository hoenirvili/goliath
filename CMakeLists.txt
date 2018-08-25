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
target_include_directories(CFGTrace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(CFGTrace PRIVATE cxx_std_17)
set_target_properties(CFGTrace
	PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
		PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
		LINK_FLAGS_DEBUG
			"/D_CRT_SECURE_NO_WARNINGS 
			/DWIN32
			/DNDEBUG
			/D_WINDOWS
			/D_USRDLL
			/DDBTTRACE_EXPORTS"
		LINK_FLAGS_RELEASE
			"/DDBTTRACE_EXPORTS
			/D_CRT_SECURE_NO_WARNINGS
			/DWIN32"
)

set(CMAKE_SYSTEM_VERSION 10.0.16299.0)
set(CMAKE_MFC_FLAG 1)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
add_subdirectory(cfgtrace)