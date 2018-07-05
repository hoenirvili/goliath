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

int ControlFlowGraph::generate(const string content, ostream* out) const noexcept
{
	(*out) << content << endl;

	auto name = to_string(this->start_address_first_node) + "_" + random_string();

	const string cmd = 
		"start \"\" cmd /c dot -Tpng partiaflowgraph.dot -o" + name + ".png 2>&1";

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


bool ControlFlowGraph::contains_address(size_t address) const noexcept
{
	for (const auto& item : this->nodes)
		if (item.second->contains_address(address))
			return true;

	return false;
}

int ControlFlowGraph::append_instruction(Instruction instruction) noexcept
{
	if (!instruction.validate())
		return EINVAL;
	
	size_t eip = instruction.pointer_address();
	if (this->start_address_first_node == 0)
		this->start_address_first_node = eip;

	if (this->current_node_start_addr == 0)
		this->current_node_start_addr = eip;
	
	unique_ptr<Node> node;

	/*if this->contains_address(this->current_node_start_addr) {
		node = this->node_that_contains_address(this->current_node_start_addr);
		
	}*/

	switch (this->node_exists(this->current_node_start_addr)) {
	// extract the node or create it
	case true:
		node = move(this->nodes[this->current_node_start_addr]);
		break;
	case false:
		node = make_unique<Node>();
		false;
	}

	node->append_instruction(instruction);
	
	if (node->done()) {
		this->current_node_start_addr = 0; // we expect a next node
		
		if (!node->true_branch_address) {
			auto true_node = make_unique<Node>();
			this->nodes[node->true_branch_address] = move(true_node);
		}

		if (!node->false_branch_address) {
			auto false_node = make_unique<Node>();
			this->nodes[node->false_branch_address] = move(false_node);
		}
	}
	
	// move the node back
	this->nodes[this->current_node_start_addr] = move(node);

	return 0;
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

int ControlFlowGraph::deserialize(const uint8_t* mem, const size_t size) noexcept
{
	if (!this->it_fits(size))
		return ENOMEM;

	const uint8_t* start = mem; // remember the start of the block

	memcpy(&this->start_address_first_node, mem, sizeof(this->start_address_first_node));
	mem += sizeof(this->start_address_first_node);

	size_t nodes_size = 0;
	memcpy(&nodes_size, mem, sizeof(nodes_size));
	mem += sizeof(nodes_size);

	this->nodes.clear(); // clear the map

	int err = 0;

	if (!nodes_size)
		log_info("deserialize every node from mem block");

	for (size_t i = 0; i < nodes_size; i++) {
		size_t start_address = 0;
		memcpy(&start_address, mem, sizeof(start_address));
		mem += sizeof(start_address);

		unique_ptr<Node> node = make_unique<Node>();
		ptrdiff_t has_written = mem - start; /*how many bytes are written*/
		err = node->deserialize(mem, size - has_written);
		if (err != 0) {
			log_error("cannot deserialize the next node");
			return err;
		}

		this->nodes[start_address] = move(node);

		mem += node->mem_size();
	}

	log_info("deserialization is finished");

	return 0;
}

int ControlFlowGraph::serialize(uint8_t *mem, size_t size) const noexcept
{
	if (!this->it_fits(size)) {
		log_error("serialise buffer cannot fit into the given size");
		return ENOMEM;
	}

	log_info("prepare memory for serialization");
	
	memset(mem, 0, size);
	
	memcpy(mem, &this->start_address_first_node, sizeof(this->start_address_first_node));
	mem += sizeof(this->start_address_first_node);

	auto n = this->nodes.size();
	memcpy(mem, &n, sizeof(n));
	mem += sizeof(n);

	log_info("start seriliazing every node in map");

	for (const auto &item : this->nodes) {
		auto addr = item.first;

		memcpy(mem, &addr, sizeof(addr));
		mem += sizeof(addr);

		size_t node_size = item.second->mem_size();
		int n = item.second->serialize(mem, node_size);
		if (n != 0) {
			log_error("cannot serialize the node");
			return n;
		}
		mem += node_size;
	}

	log_info("serialization is finished");

	return 0;
}

