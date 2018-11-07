#pragma once

#include <cfgtrace/assembly/instruction.h>
#include <cfgtrace/graph/graph.h>
#include <cfgtrace/definition/generate.h>

#include <cstddef>
#include <functional>
#include <string>

struct fake_graph : public graph::graph {
    void append(assembly::instruction instruction) override;
    void read(const std::byte *from) noexcept override;
    void write(std::byte *to) const noexcept override;
    definition::definition* generate(definition::FORMAT format) override;

    std::function<void(assembly::instruction)> _append = nullptr;
    std::function<void(const std::byte *)> _read = nullptr;
    std::function<void(std::byte *)> _write = nullptr;
    std::function<definition::definition*(definition::FORMAT)> _generate = nullptr;

    fake_graph() = default;
    ~fake_graph() = default;
};