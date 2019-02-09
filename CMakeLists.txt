cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project (Goliath
    VERSION 0.1
	DESCRIPTION "Control flow graph generator plugin"
	LANGUAGES CXX
	HOMEPAGE_URL "https://github.com/hoenirvili/goliath"
)

set(GOLIATH_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Root filepath of the project")
set(TEST_EXPORT "TEST_EXPORT")

option(BUILD_TESTS "Enable building unit tests" OFF)
option(BUILD_DEMO "Enable building demo binaries" OFF)

if (BUILD_TESTS)
	message(STATUS "Test macro is defined: ${TEST_EXPORT}")
endif()

option(GRAPHVIZ_FORMAT "Use graphviz as the format option" OFF)
option(GDL_FORMAT "Use GDL as the format option" OFF)

set(GENERATOR_TYPE_FORMAT "")
if (GRAPHVIZ_FORMAT)
    set(GENERATOR_TYPE_FORMAT "GRAPHVIZ_FORMAT")
endif()
if (GDL_FORMAT)
    set(GENERATOR_TYPE_FORMAT "GDL_FORMAT")
endif()

message(STATUS "Root directory: ${GOLIATH_ROOT_DIRECTORY}")
message(STATUS "Build test: ${BUILD_TESTS}")
message(STATUS "Build demo: ${BUILD_DEMO}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Format generator type: ${GENERATOR_TYPE_FORMAT}")

add_library(goliath goliath/goliath.cpp include/goliath.h)
target_include_directories(goliath
	PUBLIC include
	PRIVATE ${GOLIATH_ROOT_DIRECTORY})

set_target_properties(goliath
	PROPERTIES SUFFIX ".aap"
		OUTPUT_NAME "Goliath"
		ARCHIVE_OUTPUT_DIRECTORY "${GOLIATH_ROOT_DIRECTORY}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${GOLIATH_ROOT_DIRECTORY}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${GOLIATH_ROOT_DIRECTORY}/bin"
		PDB_OUTPUT_DIRECTORY "${GOLIATH_ROOT_DIRECTORY}/bin"
		COMPILE_FEATURES cxx_std_17
        COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS;$<$<BOOL:${BUILD_TESTS}>:${TEST_EXPORT}>;${GENERATOR_TYPE_FORMAT}"
        LINK_FLAGS_RELEASE "/MANIFEST:NO"
)

add_subdirectory(goliath)
print_target_properties(goliath)
add_library(goliath ALIAS goliath)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()

if (BUILD_DEMO)
	add_subdirectory(demo)
endif()