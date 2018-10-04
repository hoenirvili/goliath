add_executable(first_demo ${CMAKE_CURRENT_SOURCE_DIR}/win_version.cpp)
target_compile_features(first_demo PRIVATE cxx_std_17)
set_target_properties(first_demo
	PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		LINK_FLAGS "/MANIFEST:NO"
)

add_executable(second_demo ${CMAKE_CURRENT_SOURCE_DIR}/other.cpp)
target_compile_features(second_demo PRIVATE cxx_std_17)
set_target_properties(second_demo
	PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
		LINK_FLAGS "/MANIFEST:NO"
)