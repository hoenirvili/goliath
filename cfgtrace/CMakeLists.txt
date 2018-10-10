target_sources(cfgtrace 
	PRIVATE
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/control_flow_graph.cpp
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/control_flow_graph.h
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/instruction.cpp
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/instruction.h
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/node.cpp
		${CFGTRACE_ROOT_DIRECTORY}/cfgtrace/node.h
)

add_subdirectory(command)
add_subdirectory(engine)
add_subdirectory(error)
add_subdirectory(format)
add_subdirectory(logger)
add_subdirectory(random)
add_subdirectory(api)