find_package(Catch2 REQUIRED)
add_executable(cfgtracetest 
        main.cpp 
        cfgtrace_test.cpp
)

target_compile_features(cfgtracetest PRIVATE cxx_std_17)
target_link_libraries(cfgtracetest PRIVATE Catch2::Catch2)
target_link_libraries(cfgtracetest PRIVATE cfgtrace)
add_test(NAME cfgtracetest COMMAND cfgtracetest)