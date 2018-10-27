#pragma once

#include <ostream>
#include <sstream>

class fake_output_streamer
{
private:
    std::stringbuf buffer;

public:
    fake_output_streamer() = default;
    ~fake_output_streamer() = default;
    std::ostream *writer() noexcept;
};