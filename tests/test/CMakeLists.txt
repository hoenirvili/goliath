add_library(cfgtrace)

target_include_directories(cfgtrace
	PUBLIC ${CFGTRACE_ROOT_DIRECTORY}/tests/test/include)

set_target_properties(cfgtrace
		OUTPUT_NAME "cfgtracetestutil"
		ARCHIVE_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		PDB_OUTPUT_DIRECTORY "${CFGTRACE_ROOT_DIRECTORY}/bin"
		COMPILE_FEATURES cxx_std_17
		COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS;$<$<BOOL:${BUILD_TESTS}>:${TEST_EXPORT}>"
)

add_library(cfgtracetestutil ALIAS cfgtracetestutil)

target_link_libraries(cfgtracetest
	PRIVATE cfgtrace
	PRIVATE Catch2::Catch2
)
