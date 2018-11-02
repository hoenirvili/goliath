#include "cfgtrace/definition/generate.h"

namespace definition
{
std::string generate(generator *g, FORMAT format)
{
    return g->generate(format);
}
}; // namespace definition