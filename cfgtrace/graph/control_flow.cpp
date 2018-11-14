#include "cfgtrace/graph/control_flow.h"
#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/command/graphviz.h"
#include "cfgtrace/definition/generate.h"
#include "cfgtrace/engine/engine.h"
#include "cfgtrace/error/error.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace graph
{
std::string control_flow::graphviz()
{
    this->set_nodes_max_occurrences();

    // digraph_prefix template
    constexpr const char *digraph_prefix = R"(
digraph control_flow_graph {
	node [
		shape = box
		color = black
		arrowhead = diamond
		style = filled
		fontname = "Source Code Pro"
		arrowtail = normal
	]
)";

    std::string definitions = "";
    for (const auto &item : this->nodes)
        definitions += item.second->graphviz_definition();

    auto digraph = digraph_prefix + definitions;

    for (const auto &item : this->nodes)
        digraph += item.second->graphviz_relation();

    return digraph + "\n}";
}

void control_flow::set_nodes_max_occurrences() noexcept
{
    unsigned int max = 0;
    for (const auto &item : this->nodes)
        if (item.second->occurrences > max)
            max = item.second->occurrences;

    for (auto &item : this->nodes)
        item.second->max_occurrences = max;
}

definition::definition *control_flow::generate(definition::FORMAT format)
{
    auto engine = engine::instance();

    switch (format) {
    case definition::FORMAT::GRAPHVIZ:
        return new command::graphviz(this->graphviz(),
                                     *engine->cfg_iteration());

    case definition::FORMAT::GDL:
        throw std::invalid_argument("This is not implemented yet");
    }

    throw std::invalid_argument(
      "cannot generate definitions, unknow format specified");
}

void control_flow::write(std::byte *mem) const noexcept
{
    memcpy(mem, &this->start_address_first_node,
           sizeof(this->start_address_first_node));
    mem += sizeof(this->start_address_first_node);

    const size_t n = this->nodes.size();
    memcpy(mem, &n, sizeof(n));
    mem += sizeof(n);

    for (const auto &item : this->nodes) {
        memcpy(mem, &item.first, sizeof(item.first));
        mem += sizeof(item.first);
        item.second->load_to_memory(mem);
        mem += item.second->mem_size();
    }
}

void control_flow::read(const std::byte *mem) noexcept
{
    memcpy(&this->start_address_first_node, mem,
           sizeof(this->start_address_first_node));
    mem += sizeof(start_address_first_node);

    size_t n = 0;
    memcpy(&n, mem, sizeof(n));
    mem += sizeof(n);

    for (auto i = 0u; i < n; i++) {
        size_t key = 0;
        memcpy(&key, mem, sizeof(key));
        mem += sizeof(key);

        auto node = std::make_unique<Node>();
        node->load_from_memory(mem);
        mem += node->mem_size();

        this->nodes[key] = std::move(node);
    }
}
bool control_flow::node_exists(size_t address) const noexcept
{
    return (this->nodes.find(address) != this->nodes.end());
}

control_flow::node_ptr
control_flow::get_current_node(size_t start_address) noexcept
{
    if (this->node_exists(start_address))
        return move(this->nodes[start_address]);
    return std::make_unique<Node>(start_address);
}

bool control_flow::node_contains_address(size_t address) const noexcept
{
    for (const auto &item : this->nodes)
        if (item.second->contains_address(address))
            return true;

    return false;
}

size_t control_flow::set_and_get_current_address(size_t eip) noexcept
{
    if (this->current_node_start_addr == 0)
        this->current_node_start_addr = eip;

    if (this->current_pointer == 0)
        this->current_pointer = eip;

    return this->current_pointer;
}

void control_flow::append_instruction(assembly::instruction instruction)
{
    if (!instruction.validate())
        throw ex(std::invalid_argument, "invalid instruction passed");

    if (instruction.is_branch())
        throw ex(std::invalid_argument,
                 "cannot append instruction that is branch");

    size_t current =
      this->set_and_get_current_address(instruction.pointer_address());
    auto node = this->get_current_node(current);
    node->append_instruction(instruction);
    this->nodes[current] = move(node);
}

void control_flow::append_node_neighbors(const node_ptr &node) noexcept
{
    size_t true_address = node->true_neighbour();
    size_t false_address = node->false_neighbour();

    if (true_address != 0) {
        auto true_node = get_current_node(true_address);
        this->nodes[true_address] = move(true_node);
    }

    if (false_address != 0) {
        auto false_node = get_current_node(false_address);
        this->nodes[false_address] = move(false_node);
    }
}

void control_flow::append(assembly::instruction instruction)
{
    if (this->start_address_first_node == 0)
        this->start_address_first_node = instruction.pointer_address();

    switch (instruction.is_branch()) {
    case true:
        this->append_branch_instruction(instruction);
        break;
    case false:
        this->append_instruction(instruction);
    }
}

void control_flow::append_branch_instruction(assembly::instruction instruction)
{
    if (!instruction.validate())
        throw ex(std::invalid_argument, "invalid instruction passed");

    if (!instruction.is_branch())
        throw ex(std::invalid_argument, "cannot append non branch instruction");

    size_t current =
      this->set_and_get_current_address(instruction.pointer_address());
    auto node = this->get_current_node(current);
    node->append_branch_instruction(instruction);
    this->append_node_neighbors(node);
    this->unset_current_address(node);
    this->nodes[current] = move(node);
}

void control_flow::unset_current_address(const node_ptr &node) noexcept
{
    if (node->done())
        this->current_pointer = 0;
}

size_t control_flow::mem_size() const noexcept
{
    size_t size = sizeof(this->start_address_first_node);
    size += sizeof(size_t); // how many nodes we have
    for (const auto &item : this->nodes) {
        size += sizeof(item.first);
        size += item.second->mem_size();
    }
    return size;
}

bool control_flow::it_fits(const size_t size) const noexcept
{
    return (this->mem_size() <= size);
}

}; // namespace graph
