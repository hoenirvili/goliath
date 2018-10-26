#include "cfgtrace/logger/logger.h"
#include "fake_output_streamer.h"
#include <catch2/catch.hpp>
#include <map>
#include <memory>
#include <ostream>

TEST_CASE("Test if writer is set", "[is_writer_set]")
{
    auto state = logger::is_writer_set();
    REQUIRE_FALSE(state);
}

// TEST_CASE("Test the internal writer is set", "[is_writer_set]")
// {
//     auto fos = fake_output_streamer();
//     auto os = std::unique_ptr<std::ostream>(fos.writer());
//     logger::set_writer(os.get());
//     auto state = logger::is_writer_set();
//     REQUIRE(state == true);
// }

// TEST_CASE("Test writing to the logger with different log levels", "[write]")
// {
//     auto fos = fake_output_streamer();
//     auto os = fos.writer();
//     logger::set_writer(os);
//     auto state = logger::is_writer_set();
//     REQUIRE(state == true);

//     const std::map<logger::level, std::string> level_checks = {
//       {logger::level::info, "file:10:func |INFO| test text test\n"},
//       {logger::level::error, "file:10:func |ERROR| test text test\n"},
//       {logger::level::warning, "file:10:func |WARNING| test text test\n"},
//     };

//     for (const auto &item : level_checks) {
//         logger::write(item.first, "file", 10, "func", "test text %s", "test");
//         // fos.check(item.second);
//     }

//     logger::unset_writer();
// }

// TEST_CASE("Test if the logger is unset", "[unset_writer]")
// {
//     auto fos = fake_output_streamer();
//     auto os = fos.writer();
//     logger::set_writer(os);
//     auto state = logger::is_writer_set();
//     REQUIRE(state == true);

//     logger::unset_writer();
//     state = logger::is_writer_set();
//     REQUIRE_FALSE(state);
// }