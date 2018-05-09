#include "partial_flow_graph.hpp"
#include "common.hpp"
#include "log.hpp"
#include "types.hpp"

#include <fstream>
#include <cstddef>

using namespace std;

// digraph_prefix template
constexpr const char* digraph_prefix = R"(
digraph ControlFlowGraph {
	node [
		shape = box 
		color = black
		arrowhead = diamond
		colorscheme = blues9
		style = filled
		fontname="Source Code Pro"
		fillcolor=1
	]
}	
)";

std::string PartialFlowGraph::graphviz() const
{
    std::string definitions = "";
    for (auto& item : this->node_map) {
        auto node = item.second;
        definitions += node->graphviz_definition();
    }

    auto digraph = digraph_prefix + definitions;

    for (auto& item : this->node_map) {
        auto node = item.second;
        digraph += node->graphviz_relation();
    }

    return digraph;
}

int PartialFlowGraph::merge(const PartialFlowGraph &from) noexcept
{
	if (from.node_map.empty())
		return EINVAL;

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
		current->occurences++;
	}

	return 0;
}

int PartialFlowGraph::generate(const string content, ostream* out) const noexcept
{
	if (content.empty())
		return EINVAL;

	if (!out)
		return EINVAL;

	(*out) << content;

	const string name = to_string(this->start);
	const std::string cmd = "dot -Tpng partiaflowgraph.dot -o" + name + ".png";
	auto n = std::system(cmd.c_str());
	if (!n) {
		logger->error("cannot generate partial flow graph");
		return EINVAL;
	}

	return 0;
}

bool Instruction::is_branch() const noexcept
{
	switch (this->branch_type) {
	case JO:
	case JC:
	case JE:
	case JA:
	case JS:
	case JP:
	case JL:
	case JG:
	case JB:
	case JECXZ:
	case JmpType:
	case CallType:
	case RetType:
	case JNO:
	case JNC:
	case JNE:
	case JNA:
	case JNS:
	case JNP:
	case JNL:
	case JNG:
	case JNB:
		return true;
	}

	return false;
}

size_t Instruction::true_branch() const noexcept
{
	return this->argument_value;
}

size_t Instruction::false_branch() const noexcept
{
	return this->eip + this->len;
}

bool Instruction::validate() const
{
	if (this->content == nullptr)
		return false;

	if (this->branch_type < 0)
		return false;

	return true;
}

void PartialFlowGraph::info(const std::string & message) const noexcept
{
	if (!this->logger)
		return;

	this->logger->info(message);
}

void PartialFlowGraph::error(const std::string & message) const noexcept
{
	if (!this->logger)
		return;

	this->logger->error(message);
}

void PartialFlowGraph::warning(const std::string & message) const noexcept
{
	if (!this->logger)
		return;

	this->logger->warning(message);
}

int PartialFlowGraph::add(Instruction instruction) noexcept
{
	auto valid = instruction.validate();
	if (!valid) {
		this->error("invalid instruction passed");
		return EINVAL;
	}

	shared_ptr<Node> node = nullptr;

	if (!this->start) {
		this->info("new partial flow graph");
		this->start = instruction.eip;
	}

	if (this->should_alloc_node) {
		node = make_shared<Node>();
		node->start_address = instruction.eip;
		this->current_node_addr = instruction.eip;
		this->node_map[this->current_node_addr] = node;
		this->should_alloc_node = false;
		this->info("new partial flow graph node created");
	}

	node = this->node_map[this->current_node_addr];

	if (instruction.is_branch()) {
		this->info("partial flow graph node is branching");
		node->true_branch_address = instruction.true_branch();
		node->false_branch_address = instruction.false_branch();
		this->should_alloc_node = true;
	}
		
	node->block.push_back(instruction.content);

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
	
	return size + sizeof(this->guard); /*with the guard value*/
}

bool PartialFlowGraph::it_fits(const size_t size) const noexcept
{
	return (size >= this->mem_size());
}

int PartialFlowGraph::deserialize(const uint8_t* mem, const size_t size) noexcept
{
	if ((!mem) || (!size))
		return EINVAL;

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
		this->info("deserialize every node from mem block");
	
	for (size_t i = 0; i < node_map_size; i++) {
		size_t start_address = 0;
		memcpy(&start_address, mem, sizeof(start_address));
		mem += sizeof(start_address);

		shared_ptr<Node> node = make_shared<Node>();
		ptrdiff_t has_written = mem - start; /*how many bytes are written*/
		err = node->deserialize(mem, size-has_written);
		if (err != 0) {
			this->error("cannot deserialize the next node");
			return err;
		}

		this->node_map[start_address] = node;

		mem += node->mem_size();
	}

	uint16_t guard = 0;
	memcpy(&guard, mem, sizeof(guard));
	if (this->guard != guard)
		return EINVAL;

	this->info("deserialization is finished, guard value checked");

	return 0;
}


int PartialFlowGraph::serialize(uint8_t *mem, size_t size) const noexcept
{
	if ((!mem) || (!size)) {
		this->error("invalid shared block of memory provided");
		return EINVAL;
	}

	if (!this->it_fits(size)) {
		this->error("serialise buffer cannot fit into the given size");
		return ENOMEM;
	}
	
	this->info("prepare memory for serialization");
	memset(mem, 0, size);

	/**
	* serialisation on x86
	* - first 4 bytes is the start addrs of the first node node_map
	* - second 4 bytes are the number of nodes
	* - after we start in pairs: (addr, node)
	*	- first 4 bytes addr node
	*	- second node->mem_size() bytes
	* - we end with the random guard value of 0x7777
	*/
	memcpy(mem, &this->start, sizeof(this->start));
	mem += sizeof(this->start);

	auto n = this->node_map.size();
	memcpy(mem, &n, sizeof(n));
	mem += sizeof(n);

	this->info("start seriliazing every node in map");
	for (const auto &item : this->node_map) {
		auto addr = item.first;

		memcpy(mem, &addr, sizeof(addr));
		mem += sizeof(addr);

		auto node = item.second;
		
		size_t node_size = node->mem_size();
		int n = node->serialize(mem, node_size);
		if (n != 0) {
			this->error("cannot serialize the node");
			return n;
		}
		mem += node_size;
	}

	memcpy(mem, &this->guard, sizeof(this->guard));
	this->info("serialization is finished, guard value added");

	return 0;
}

