#include "fake_graph.h"
#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/graph/graph.h"
#include <cstddef>

void fake_graph::append(assembly::instruction instruction)
{
    if (!this->_append)
        return;

    this->_append(instruction);
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