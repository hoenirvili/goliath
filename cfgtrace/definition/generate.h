#pragma once

#include <string>

namespace definition
{
enum class FORMAT : uint8_t { GRAPHVIZ, GDL };

struct definition {
    virtual ~definition() = default;

    virtual void execute() const = 0;
};

struct generator {
    virtual ~generator() = default;

    virtual definition *generate(FORMAT format) = 0;
};

definition *generate(generator *g, FORMAT format);

}; // namespace definition