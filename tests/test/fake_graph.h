#pragma once

#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/graph/graph.h"
#include <cstddef>
#include <functional>

struct fake_graph : public graph::graph {
    void append(assembly::instruction instruction) override;
    void read(const std::byte *from) noexcept override;
    void write(std::byte *to) const noexcept override;

    std::function<void(assembly::instruction)> _append = nullptr;
    std::function<void(const std::byte *)> _read = nullptr;
    std::function<void(std::byte *)> _write = nullptr;

    fake_graph() = default;
    ~fake_graph() = default;
};