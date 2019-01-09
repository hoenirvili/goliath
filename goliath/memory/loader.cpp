#include "loader.h"
#include <cstddef>

namespace memory
{
void loader(reader *r, const std::byte *from) noexcept
{
    r->read(from);
}
}; // namespace memory