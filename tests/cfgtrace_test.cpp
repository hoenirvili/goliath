#include "cfgtrace/logger/logger.h"
#include "fake_output_streamer.h"
#include "virtual_memory.h"
#include <catch2/catch.hpp>
#include <cfgtrace.h>

TEST_CASE("The plugin is assumed to be run in layer 2", "[GetLayer]")
{
    size_t layer = GetLayer();
    REQUIRE(layer == PLUGIN_LAYER);
}

TEST_CASE("When the internal virtual memory is not initialised", "[DBTInit]")
{
    BOOL state = DBTInit();
    REQUIRE(state == FALSE);
}

TEST_CASE("When the internal virtual memory is initialised", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();

    SECTION("file_mapping is now constructed but log name is not available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == FALSE);
    }
}

TEST_CASE("When the internal virtual memory log is initialised", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();
    vm.enable_log_name();
    fake_output_streamer fom = fake_output_streamer();
    std::ostream *w = fom.writer();
    logger::init(w);

    SECTION("file_mapping is constructed and log name is available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == TRUE);
        fom.check("[CFGTrace] Init is called");
        fom.check("[CFGTrace] [CFGTrace] Iinit is called for iteration 1");
    }

    logger::clean();
}
