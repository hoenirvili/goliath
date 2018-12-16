#pragma once

#include <string>

namespace definition
{
enum class FORMAT : uint8_t { GRAPHVIZ, GDL };

struct stringer {
    virtual ~stringer() = default;

    virtual std::string_view string() const = 0;
};

struct definition : public stringer {
    virtual ~definition() = default;

    virtual void execute() const = 0;
};

struct generator {
    virtual ~generator() = default;

    virtual definition *generate(FORMAT format) = 0;
};

definition *generate(generator *g, FORMAT format);

}; // namespace definition