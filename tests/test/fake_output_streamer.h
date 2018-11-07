#pragma once

#include <ostream>
#include <sstream>

class fake_output_streamer
{
public:
    const char *name() const noexcept;
    std::ostream *writer(const char *name);
    void fake_output_streamer::contains(const char *str);

    fake_output_streamer() = default;
    ~fake_output_streamer();
private:
    std::stringbuf buffer;
    std::ostream *os = nullptr;

    const char *_name = nullptr;
    void init_only_once() noexcept;
};