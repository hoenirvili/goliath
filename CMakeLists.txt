cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project (CFGTrace
	VERSION 0.1
	DESCRIPTION "Control flow graph generator plugin"
	LANGUAGES CXX
	HOMEPAGE_URL "https://github.com/hoenirvili/cfgtrace"
)

function(print_target_properties tgt)
	if(NOT TARGET ${tgt})
		message("There is no target named '${tgt}'")
		return()
	endif()
	set(CMAKE_PROPERTY_LIST
		OUTPUT_NAME
		SUFFIX
		SOURCE_DIR
		BINARY_DIR
		ARCHIVE_OUTPUT_DIRECTORY
		LIBRARY_OUTPUT_DIRECTORY
		RUNTIME_OUTPUT_DIRECTORY
		PDB_OUTPUT_DIRECTORY
		COMPILE_DEFINITIONS
		COMPILE_OPTIONS
		INCLUDE_DIRECTORIES
		LINK_LIBRARIES
		LINK_FLAGS)
	message("Configuration for target ${tgt}")
	foreach (prop ${CMAKE_PROPERTY_LIST})
		get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
		if (propval)
			get_target_property(propval ${tgt} ${prop})
			message (STATUS "${prop} = ${propval}")
		endif()
	endforeach(prop)
endfunction(print_target_properties)

set(CFGTRACE_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Root filepath of the project")
set(TEST_EXPORT "TEST_EXPORT")

option(BUILD_TESTS "Enable building unit tests" OFF)
option(BUILD_DEMO "Enable building demo binaries" OFF)

if (BUILD_TESTS)
	message(STATUS "Test macro is defined: ${TEST_EXPORT}")
endif()

message(STATUS "Root directory: ${CFGTRACE_ROOT_DIRECTORY}")
message(STATUS "Build test: ${BUILD_TESTS}")
message(STATUS "Build demo: ${BUILD_DEMO}")
message(STATUS "Build type:  ${CMAKE_BUILD_TYPE}")


add_library(cfgtrace cfgtrace/cfgtrace.cpp include/cfgtrace.h)
target_include_directories(cfgtrace
	PUBLIC include
	PRIVATE ${CFGTRACE_ROOT_DIRECTORY})

set_target_properties(cfgtrace
	PROPERTIES SUFFIX ".aap"
		OUTPUT_NAME "CFGTrace"
		ARCHIVE_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		PDB_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		COMPILE_FEATURES cxx_std_17
		COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS;$<$<BOOL:${BUILD_TESTS}>:${TEST_EXPORT}>"
)

add_subdirectory(cfgtrace)
print_target_properties(cfgtrace)
add_library(cfgtrace ALIAS cfgtrace)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()

if (BUILD_DEMO)
	add_subdirectory(demo)
endif()