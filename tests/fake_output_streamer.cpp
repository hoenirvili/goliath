#include "fake_output_streamer.h"
#include <catch2/catch.hpp>
#include <ostream>
#include <string_view>

std::ostream *fake_output_streamer::writer() noexcept
{
    return new std::ostream(&this->buffer);
}