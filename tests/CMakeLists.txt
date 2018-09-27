enable_testing()
find_package(Catch2 REQUIRED)
add_executable(cfgtracetest main.cpp)
target_compile_features(cfgtracetest PRIVATE cxx_std_17)
target_link_libraries(cfgtracetest Catch2::Catch2)
add_test(CFGTracetest cfgtracetest
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/tests/"
)