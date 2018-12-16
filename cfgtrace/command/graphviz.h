#pragma once

#include "cfgtrace/definition/generate.h"

#include <string>

namespace command
{
struct graphviz : public definition::definition {
    graphviz(std::string_view definitions, size_t iteration);
    ~graphviz() = default;

    void execute() const override;
    std::string_view string() const override;

private:
    std::string definitions;
    size_t iteration;
};

}; // namespace command