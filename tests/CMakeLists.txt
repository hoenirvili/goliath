enable_testing()

find_package(Catch2 REQUIRED)

add_executable(cfgtracetest
	main.cpp
	cfgtrace.cpp
    logger.cpp
)
add_subdirectory(test)

target_include_directories(cfgtracetest
	PRIVATE ${CFGTRACE_ROOT_DIRECTORY})

target_link_libraries(cfgtracetest
	PRIVATE cfgtrace
    PRIVATE Catch2::Catch2
)

get_target_property(OUTPUT cfgtrace RUNTIME_OUTPUT_DIRECTORY)

set_target_properties(cfgtracetest
	PROPERTIES
		COMPILE_FEATURES cxx_std_17
		RUNTIME_OUTPUT_DIRECTORY ${OUTPUT}
)

add_test(NAME cfgtracetest COMMAND cfgtracetest)