#pragma once

#include <cstddef>

namespace memory
{
struct writer {
    virtual void write(std::byte *to) const noexcept = 0;
    virtual ~writer() = default;
};

void unloader(writer *w, std::byte *to) noexcept;
}; // namespace memory
