#include "fake_output_streamer.h"
#include <catch2/catch.hpp>
#include <ostream>

std::ostream *fake_output_streamer::writer() noexcept
{
    return new std::ostream(&this->buffer);
}

void fake_output_streamer::check(std::string_view message) noexcept
{
    std::string buff = this->buffer.str();
    auto n = buff.find(message, 0);
    REQUIRE(n != std::string::npos);
};