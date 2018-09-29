#include <catch2/catch.hpp>
#include <cfgtrace.h>
#include <cfgtrace/api/types.h>

TEST_CASE("The plugin is assumed to be run in layer 2", "[GetLayer]")
{
    size_t layer = GetLayer();
    REQUIRE(layer == PLUGIN_LAYER);
}