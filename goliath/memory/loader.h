#pragma once

#include <cstddef>

namespace memory
{
struct reader {
    virtual void read(const std::byte *from) noexcept = 0;
    virtual ~reader() = default;
};

void loader(reader *r, const std::byte *from) noexcept;

}; // namespace memory