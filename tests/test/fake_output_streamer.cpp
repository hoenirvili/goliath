#include "fake_output_streamer.h"
#include <catch2/catch.hpp>
#include <ostream>

using Catch::Matchers::Contains;

std::ostream *fake_output_streamer::writer(const char *name)
{
    this->_name = name;
    this->init_only_once();
    return this->os;
}

void fake_output_streamer::init_only_once() noexcept
{
    if (this->os)
        return;

    this->os = new std::ostream(&this->buffer);
}

const char *fake_output_streamer::name() const noexcept
{
    return this->_name;
}

void fake_output_streamer::contains(const char *str)
{
    auto data = buffer.str();
    REQUIRE_THAT(data, Contains(str));
}