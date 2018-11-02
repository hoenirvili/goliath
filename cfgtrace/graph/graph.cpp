#include "cfgtrace/graph/graph.h"
#include "cfgtrace/graph/control_flow.h"

namespace graph
{
/**
 * g is the global internal graph in this module
 * all actions performed from the public api will modify this instance
 */
static graph *g;

/**
 *  fn internal creator function for customizing
 *  the creation of a graph
 */
static creator fn;

/**
 *  graph_creator create a default graph
 *  using one of the available graph implementations
 */
static graph *graph_creator()
{
    return new control_flow();
}

bool is_initialised() noexcept
{
    return (g);
}

graph *instance() noexcept
{
    if (g)
        return g;

    if (!fn)
        fn = graph_creator;

    if (!g)
        g = fn();

    return g;

}

void clean() noexcept
{
    if (!g)
        return;

    delete g;
    g = nullptr;
}

void custom_creation(creator create) noexcept
{
    fn = create;
}

}; // namespace graph