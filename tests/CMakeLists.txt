enable_testing()
find_package(Catch2 REQUIRED)
add_executable(cfgtracetest
	main.cpp
	cfgtrace_test.cpp
)
target_include_directories(cfgtracetest
	PRIVATE ${CFGTRACE_ROOT_DIRECTORY})

include(CTest)
include(Catch)
catch_discover_tests(cfgtracetest)

target_compile_features(cfgtracetest PRIVATE cxx_std_17)
target_link_libraries(cfgtracetest
	PRIVATE Catch2::Catch2
	PRIVATE cfgtrace)
target_compile_options(cfgtracetest PRIVATE "/MD")
get_target_property(OUTPUT cfgtrace RUNTIME_OUTPUT_DIRECTORY)
set_target_properties(cfgtracetest
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${OUTPUT})

add_test(NAME cfgtracetest COMMAND cfgtracetest)