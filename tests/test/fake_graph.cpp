#include "fake_graph.h"

#include <goliath/assembly/instruction.h>
#include <goliath/definition/generate.h>
#include <goliath/graph/graph.h>

#include <cstddef>
#include <string>

void fake_graph::append(assembly::instruction instruction, size_t iteration)
{
    if (!this->_append)
        return;

    this->_append(instruction, iteration);
}

void fake_graph::read(const std::byte *from) noexcept
{
    if (!this->_read)
        return;
    this->_read(from);
}

void fake_graph::write(std::byte *to) const noexcept
{
    if (!this->_write)
        return;
    this->_write(to);
}

definition::definition *fake_graph::generate(definition::FORMAT format)
{
    if (!this->_generate)
        return nullptr;

    return this->_generate(format);
}