#pragma once

#include "cfgtrace/definition/generate.h"

#include <string>

namespace command
{
/**
 * graphviz type used to store all graphviz context information
 * for generating a graph
 */
struct graphviz : public definition::definition {
    /**
     * create a new graphviz graph at a certain iteration based
     * on the definitions provided
     */
    graphviz(std::string_view definitions, size_t iteration);
    ~graphviz() = default;

    /**
     * execute compiles the graphviz graph into a png file
     */
    void execute() const override;

    /**
     * return the graphviz format definitions as a string
     */
    std::string_view string() const override;

private:
    std::string definitions;
    size_t iteration;
};

}; // namespace command