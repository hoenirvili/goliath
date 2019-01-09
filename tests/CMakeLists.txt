enable_testing()

find_package(Catch2 REQUIRED)

add_executable(goliathtest
	main.cpp
	goliath.cpp
    run.cpp
    machinery.cpp
    machinery.h
)
add_subdirectory(test)

target_include_directories(goliathtest
	PRIVATE ${GOLIATH_ROOT_DIRECTORY})

target_link_libraries(goliathtest
	PRIVATE goliath
    PRIVATE Catch2::Catch2
)

get_target_property(OUTPUT goliath RUNTIME_OUTPUT_DIRECTORY)

set_target_properties(goliathtest
	PROPERTIES
		COMPILE_FEATURES cxx_std_17
		RUNTIME_OUTPUT_DIRECTORY ${OUTPUT}
)

add_test(NAME goliathtest COMMAND goliathtest)