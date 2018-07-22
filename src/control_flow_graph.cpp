#include <fstream>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "control_flow_graph.h"
#include "api.h"
#include "log.h"
#include "instruction.h"
#include "random.h"
#include "exec.h"

using namespace std;

// digraph_prefix template
constexpr const char* digraph_prefix = R"(
digraph ControlFlowGraph {
	node [
		shape = box 
		color = black
		arrowhead = diamond
		style = filled
		fontname = "Source Code Pro"
		arrowtail = normal
	]	
)";

string ControlFlowGraph::graphviz()
{

	this->set_nodes_max_occurrences();

	string definitions = "";
	for (const auto& item : this->nodes) 
		definitions += item.second->graphviz_definition();

	auto digraph = digraph_prefix + definitions;

	for (const auto& item : this->nodes)
		digraph += item.second->graphviz_relation();

	return digraph + "\n}";
}

int ControlFlowGraph::serialize(uint8_t * mem, size_t size) const noexcept
{
	return 0;
}

int ControlFlowGraph::deserialize(const uint8_t * mem, size_t size) noexcept
{
	return 0;
}

int ControlFlowGraph::generate(const string content, ostream* out) const noexcept
{
	(*out) << content << endl;

	auto name = to_string(this->start_address_first_node) + "_" + random_string();

	const string cmd = "dot -Tpng partiaflowgraph.dot -o" + name + ".png";
	auto from = execute_command(cmd);
	if (!from.empty())
		log_error(
			"\n[DOT COMMAND OUTPUT START]\n %s \n[DOT COMMAND OUTPUT END]",
			from.c_str()
		);

	return 0;
}

bool ControlFlowGraph::node_exists(size_t address) const noexcept
{
	return (this->nodes.find(address) != this->nodes.end());
}

unique_ptr<Node> ControlFlowGraph::get_current_node(size_t start_address) noexcept
{
	if (this->node_exists(start_address))
		return move(this->nodes[start_address]);
	return make_unique<Node>(start_address);
}

bool ControlFlowGraph::node_contains_address(size_t address) const noexcept
{
	for (const auto& item : this->nodes)
		if (item.second->contains_address(address))
			return true;

	return false;
}

size_t ControlFlowGraph::set_and_get_current_address(size_t eip) noexcept
{
	if (this->current_node_start_addr == 0)
		this->current_node_start_addr = eip;

	if (this->current_pointer == 0)
		this->current_pointer = eip;
	
	return this->current_pointer;
}

int ControlFlowGraph::append_instruction(Instruction instruction) noexcept
{
	if (!instruction.validate())
		return EINVAL;
	
	if (instruction.is_branch())
		return 0;

	size_t current = this->set_and_get_current_address(instruction.pointer_address());
	auto node = this->get_current_node(current);
	node->append_instruction(instruction);
	this->nodes[current] = move(node);
	
	return 0;
}

void ControlFlowGraph::append_node_neighbours(const unique_ptr<Node>& node) noexcept
{
	size_t true_address = node->true_neighbour();
	size_t false_address = node->false_neighbour();

	if (true_address != 0) {
		auto tnode = get_current_node(true_address);
		this->nodes[true_address] = move(tnode);
	}

	if (false_address != 0) {
		auto fnode = get_current_node(false_address);
		this->nodes[false_address] = move(fnode);
	}
}

int ControlFlowGraph::append_branch_instruction(Instruction instruction) noexcept
{
	if (!instruction.validate())
		return EINVAL;
	
	size_t current = this->set_and_get_current_address(instruction.pointer_address());
	auto node = this->get_current_node(current);
	node->append_branch_instruction(instruction);
	this->append_node_neighbours(node);
	this->unset_current_address(node);
	this->nodes[current] = move(node);

	return 0;
}

void ControlFlowGraph::unset_current_address(const unique_ptr<Node>& node) noexcept
{
	if (node->done())
		this->current_pointer = 0;
}

size_t ControlFlowGraph::mem_size() const noexcept
{
	size_t size = 0;
	size += sizeof(this->start_address_first_node);
	size += sizeof(this->nodes.size());

	for (const auto &item : this->nodes) {
		const auto address = item.first;

		size += sizeof(address);
		size += item.second->mem_size();
	}

	return size;
}

void ControlFlowGraph::set_nodes_max_occurrences() noexcept
{
	unsigned int max = 0;
	for (const auto &item : this->nodes)
		if (item.second->occurrences > max)
			max = item.second->occurrences;

	for (auto &item : this->nodes)
		item.second->max_occurrences = max;
}

bool ControlFlowGraph::it_fits(const size_t size) const noexcept
{
	return (this->mem_size() <= size);
}