#include "goliath/definition/generate.h"

namespace definition
{
definition *generate(generator *g, FORMAT format)
{
    return g->generate(format);
}

}; // namespace definition