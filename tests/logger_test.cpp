#include "cfgtrace/logger/logger.h"
#include "fake_output_streamer.h"
#include <catch2/catch.hpp>
#include <memory>
#include <ostream>

TEST_CASE("Test if writer is set", "[is_writer_set]")
{
    auto state = logger::is_writer_set();
    REQUIRE_FALSE(state);
}

TEST_CASE("Test the internal writer is set", "[is_writer_set]")
{
    auto fos = fake_output_streamer();
    auto os = std::unique_ptr<std::ostream>(fos.writer());
    logger::set_writer(os.get());
    auto state = logger::is_writer_set();
    REQUIRE(state == true);
}

TEST_CASE("Test writing to the logger", "[write]")
{
    auto fos = fake_output_streamer();
    auto os = fos.writer();
    logger::set_writer(os);
    auto state = logger::is_writer_set();
    REQUIRE(state == true);

    logger::write(logger::level::info, "file", 10, "func", "test text %s", "test");
    fos.check("file:10:func |INFO| test text test\n");

    logger::unset_writer();
}

TEST_CASE("Test if the logger is unset", "[unset_writer]")
{
    auto fos = fake_output_streamer();
    auto os = fos.writer();
    logger::set_writer(os);
    auto state = logger::is_writer_set();
    REQUIRE(state == true);

    logger::unset_writer();
    state = logger::is_writer_set();
    REQUIRE_FALSE(state);
}
