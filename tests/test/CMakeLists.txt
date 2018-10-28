target_sources(cfgtracetest
    PRIVATE
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/catch_extra_matchers.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/custom_params.cpp
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/custom_params.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/fake_output_streamer.cpp
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/fake_output_streamer.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/plugin_layer.cpp
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/plugin_layer.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/plugin_report.cpp
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/plugin_report.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/virtual_memory.cpp
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/virtual_memory.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/fake_graph.h
        ${CFGTRACE_ROOT_DIRECTORY}/tests/test/fake_graph.cpp

)