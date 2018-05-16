#include <fstream>
#include <cstddef>
#include <string>

#include "partial_flow_graph.hpp"
#include "common.hpp"
#include "log.hpp"
#include "types.hpp"
#include "instruction.hpp"

using namespace std;

// digraph_prefix template
constexpr const char* digraph_prefix = R"(
digraph ControlFlowGraph {
	node [
		shape = box 
		color = black
		arrowhead = diamond
		style = filled
		fontname="Source Code Pro"
	]	
)";

string PartialFlowGraph::graphviz() const
{
	string definitions = "";
    for (const auto& item : this->node_map) {
        auto node = item.second;
        definitions += node->graphviz_definition();
    }

    auto digraph = digraph_prefix + definitions;

    for (const auto& item : this->node_map) {
        auto node = item.second;
		auto relation = node->graphviz_relation();
		if (!relation.empty())
			digraph += node->graphviz_relation();
    }

	return digraph + "\n}";
}

int PartialFlowGraph::merge(const PartialFlowGraph &from) noexcept
{
	if (from.node_map.empty())
		return 0;

	if (this->start != from.start)
		return EINVAL;

	for (const auto &item : from.node_map) {
		auto key = item.first;
		auto node = item.second;

		// if the key is not in the map
		if (this->node_map.find(key) == this->node_map.end()) {
			this->node_map[key] = node;
			continue;
		}

		auto current = this->node_map[key];
		current->occurrences++;
	}

	return 0;
}


int PartialFlowGraph::generate(const string content, ostream* out) const noexcept
{
	(*out) << content << endl;
	
	auto name = to_string(this->start) + "_" + random_string();

	const string cmd = "start \"\" cmd /c dot -Tpng partiaflowgraph.dot -o" + 
			name + ".png";

	auto from = execute_command(cmd);
	if (!from.empty())
		log_error(
			"\n[DOT COMMAND OUTPUT START]\n %s \n[DOT COMMAND OUTPUT END]", 
			from.c_str()
		);
	
	return 0;
}

int PartialFlowGraph::add(const Instruction& instruction) noexcept
{
	if (!instruction.validate()) {
		log_error("invalid instruction passed");
		return EINVAL;
	}

	auto address = instruction.eip;

	// at the start of the partial flow graph init address
	if (!this->start)
		this->start = address;


	auto found = this->node_map.find(address) != this->node_map.end();
	
	// if this is true, this means we have to create a new node
	if (!found && !this->ret_instr_encountered) {
		auto new_node = make_shared<Node>();
		new_node->start_address = address;
		this->node_map[address] = new_node;

		this->current_node_addr = address;
	}

	if (this->ret_instr_encountered) {
		// we already created the node 
		this->current_node_addr = address;
		this->ret_instr_encountered = false;
	}

	auto current_node = this->node_map[this->current_node_addr];

	if (instruction.is_branch() && 
		instruction.branch_type != RetType) {
		
		// finish with the old node
		auto taddr = instruction.true_branch();
		auto faddr = instruction.false_branch();
		current_node->true_branch_address = taddr;
		current_node->false_branch_address = faddr;

		// we know that every false and true branch could
		// point to new nodes so create, init and push them
		// into the map
		if (taddr != address) {
			auto tnode = make_shared<Node>();
			tnode->start_address = taddr;
			this->node_map[taddr] = tnode;
		}
		if (faddr != address) {
			auto fnode = make_shared<Node>();
			fnode->start_address = faddr;
			this->node_map[faddr] = fnode;
		}
	}

	if (instruction.branch_type == RetType) {
		this->ret_instr_encountered = true;
	}


	// every instruction pushed should be in the node
	current_node->block.push_back(instruction.content);
	
	return 0;
}

size_t PartialFlowGraph::mem_size() const noexcept
{
	size_t size = 0;
	size += sizeof(this->start);
	size += sizeof(this->node_map.size());
	
	for (const auto &item : this->node_map) {
		const auto address = item.first;
		const auto node = item.second;
		
		size += sizeof(address);
		size += node->mem_size();
	}
	
	return size;
}

bool PartialFlowGraph::it_fits(const size_t size) const noexcept
{
	return (size >= this->mem_size());
}


int PartialFlowGraph::deserialize(const uint8_t* mem, const size_t size) noexcept
{
	if (!this->it_fits(size))
		return ENOMEM;

	const uint8_t* start = mem; // remember the start of the block

	memcpy(&this->start, mem, sizeof(this->start));
	mem += sizeof(this->start);

	size_t node_map_size = 0;
	memcpy(&node_map_size, mem, sizeof(node_map_size));
	mem += sizeof(node_map_size);
	
	this->node_map.clear(); // clear the map

	int err = 0;
	
	if (!node_map_size)
		log_info("deserialize every node from mem block");
	
	for (size_t i = 0; i < node_map_size; i++) {
		size_t start_address = 0;
		memcpy(&start_address, mem, sizeof(start_address));
		mem += sizeof(start_address);

		shared_ptr<Node> node = make_shared<Node>();
		ptrdiff_t has_written = mem - start; /*how many bytes are written*/
		err = node->deserialize(mem, size-has_written);
		if (err != 0) {
			log_error("cannot deserialize the next node");
			return err;
		}

		this->node_map[start_address] = node;

		mem += node->mem_size();
	}

	log_info("deserialization is finished");

	return 0;
}


int PartialFlowGraph::serialize(uint8_t *mem, size_t size) const noexcept
{
	if ((!mem) || (!size)) {
		log_error("invalid shared block of memory provided");
		return EINVAL;
	}

	if (!this->it_fits(size)) {
		log_error("serialise buffer cannot fit into the given size");
		return ENOMEM;
	}
	
	log_info("prepare memory for serialization");
	memset(mem, 0, size);

	/**
	* serialization on x86
	* - first 4 bytes is the start addrs of the first node node_map
	* - second 4 bytes are the number of nodes
	* - after we start in pairs: (addr, node)
	*	- first 4 bytes addr node
	*	- second node->mem_size() bytes
	*/
	memcpy(mem, &this->start, sizeof(this->start));
	mem += sizeof(this->start);

	auto n = this->node_map.size();
	memcpy(mem, &n, sizeof(n));
	mem += sizeof(n);

	log_info("start seriliazing every node in map");
	for (const auto &item : this->node_map) {
		auto addr = item.first;

		memcpy(mem, &addr, sizeof(addr));
		mem += sizeof(addr);

		auto node = item.second;
		
		size_t node_size = node->mem_size();
		int n = node->serialize(mem, node_size);
		if (n != 0) {
			log_error("cannot serialize the node");
			return n;
		}
		mem += node_size;
	}

	log_info("serialization is finished");

	return 0;
}

