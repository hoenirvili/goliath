find_package(Catch2 REQUIRED)
add_executable(cfgtracetest 
        main.cpp 
        cfgtrace_test.cpp
)

target_compile_features(cfgtracetest PRIVATE cxx_std_17)
target_link_libraries(cfgtracetest PRIVATE Catch2::Catch2)
target_link_libraries(cfgtracetest PRIVATE cfgtrace)

# make the RUNTIME_OUTPUT_DIRECTORY the same with the main SHARED lib location
get_target_property(OUTPUT cfgtrace RUNTIME_OUTPUT_DIRECTORY)
set_target_properties(cfgtracetest PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT})

add_test(NAME cfgtracetest COMMAND cfgtracetest)