#pragma once

#include <string>

namespace definition
{

enum class FORMAT : uint8_t { GRAPHVIZ, GDL };

struct generator {
    virtual ~generator() = default;

    virtual std::string generate(FORMAT format) const = 0;
};

std::string generate(generator *g, FORMAT format);

}; // namespace definition