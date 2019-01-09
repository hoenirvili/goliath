#include "unloader.h"
#include <cstddef>

namespace memory
{
void unloader(writer *w, std::byte *to) noexcept
{
    w->write(to);
}
}; // namespace memory