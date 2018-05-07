#include "partial_flow_graph.hpp"

#include "common.hpp"
#include "log.hpp"
#include "types.hpp"

#include <fstream>
#include <assert.h>

using namespace std;

using NodeMap = std::map<size_t, std::unique_ptr<Node>>;

//// _merge iterates in *from* and if the element exists in *into*
//// increment the occurences fiels or move the element in *into*
//static void _merge(const NodeMap from, NodeMap& into)
//{
//    for (auto& item : from) {
//        auto key = item.first;
//        auto value = item.second;
//
//        auto it = into.find(key);
//        // no key found
//        if (it == into.end()) {
//            into[key] = value;
//            continue;
//        }
//        // key found
//        into[key]->occurences++;
//    }
//}
//
//// merge merges all NodeMaps from storage and returns it
//NodeMap PartialFlowGraph::merge()
//{
//    assert(storage.empty());
//    auto into = storage[0];
//    vector<int>::size_type i = 1;
//    for (; i != storage.size(); i++)
//        _merge(storage[i], into);
//
//    return into;
//}
//
//// digraph_prefix template
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
//
//PartialFlowGraph::~PartialFlowGraph()
//{
//
//    // TODO:
//    // HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
//    // BYTE *shared_mem = (BYTE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
//    // memset(CFG(shared_mem), 0, 0x8000);
//    // memcpy(CFG(shared_mem), formula.c_str(), formula.size());
//    // CloseHandle(hMapFile);
//
//    this->nl = this->merge();
//    auto graphviz = this->graphviz();
//    this->generate(graphviz, "final");
//
//    // clean up
//    for (auto& nodelist : storage)
//        for (auto& node : nodelist)
//            delete node.second;
//}
//
//std::string PartialFlowGraph::graphviz()
//{
//    assert(this->nl.size() != 0);
//    assert(this->start != 0);
//
//    std::string definitions = "";
//    for (auto& item : this->nl) {
//        auto node = item.second;
//        definitions += node->graphviz_definition();
//    }
//
//    auto digraph = digraph_prefix + definitions;
//
//    for (auto& item : this->nl) {
//        auto node = item.second;
//        digraph += node->graphviz_relation();
//    }
//
//    return digraph;
//}
//
//void PartialFlowGraph::generate(std::string content, std::string fname)
//{
//    if (content.empty()) {
//        return;
//    }
//
//    const auto prefix = ".dot";
//    if (fname.empty())
//        fname = std::to_string(this->start);
//
//    auto file = fstream(fname+prefix);
//    file << content;
//    file.close();
//
//    const std::string cmd = "dot -Tpng " + fname + prefix + " -o" + fname + ".png";
//    auto n = std::system(cmd.c_str());
//	//if (!n)
//		//logger.error("cannot generate partial flow graph");
//}
//

static inline bool is_branch(BRANCH_TYPE t)
{
    
}

//void PartialFlowGraph::add_instruction(Instruction instruction)
//{
//    if (this->start == instruction.eip) {
//        // these fields can't be 0
//        assert(this->nl.size() != 0);
//        assert(this->start != 0);
//        assert(this->next_instr != 0);
//        /**
//         *after the last node has been added in our
//         *partial flow graph, push back the graph
//         */
//        storage.push_back(this->nl);
//
//        /**
//         *generate a control flow graph per every graph in png format
//         */
//        auto graphviz = this->graphviz();
//        this->generate(graphviz);
//        /**
//         *restart the internal state
//         */
//        this->nl.clear();    // clear the node list
//        this->start = 0;
//        this->next_instr = 0;
//    }
//
//    Node* node;
//
//    if (!this->start)
//        this->start = instruction.eip;
//
//    if (!start_instr)
//        start_instr = this->start;
//
//    if (!this->next_instr) {
//        this->next_instr = instruction.eip;
//        auto n = new Node();
//        n->start_address = this->next_instr;
//        this->nl[this->next_instr] = n;
//    }
//
//    node = this->nl[this->next_instr];
//
//    /*always test if the current instruction is branching*/
//    if (is_branch(instruction.branch_type)) {
//        node->true_branch_address = instruction.argument_value; /*jump to next node*/
//        node->false_branch_address = instruction.eip + instruction.len; /*point to the next instruction*/
//        this->next_instr = 0; /*we have a new node*/
//    }
//
//    node->block.push_back(instruction.content);
//}

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

void PartialFlowGraph::add(Instruction instruction) noexcept
{
	auto valid = instruction.validate();
	if (!valid) {
		this->error("Invalid instruction passed");
		return;
	}

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

bool PartialFlowGraph::it_fits(size_t into) const noexcept
{
	auto n = this->mem_size();
	return (n <= into);
}

int PartialFlowGraph::serialize(uint8_t *mem, size_t size) const noexcept
{
	if ((!mem) || (!size)) {
		this->error("invalid shared block of memory provided");
		return EINVAL;
	}

	if (!this->it_fits(size)) {
		this->error("serialise buffer can't fit into the given size");
		return EINVAL;
	}
	
	memset(mem, 0, size);
	this->info("prepare memory for serialization");
	/**
	* serialisation on x86
	* - first 4 bytes is the start addrs of the first node node_map
	* - second 4 bytes are the number of nodes
	* - after we start in pairs:
	*	- first 4 bytes are the start_addr of the node
	*	- second 4 bytes are the number of occurences
	*	- third 4 bytes true branch address
	*	- forth 4 bytes false branch address
	*	- fifth 4 bytes is the size of the node block
	*	- the last is the hole block as strings, every string  is separated by "\0"
	*	so we don't need to include his size also
	* - we end with the random guard value of 0x7777
	*/
	memcpy(mem, &this->start, sizeof(this->start));
	mem += sizeof(this->start);
	
	auto n = this->node_map.size();
	memcpy(mem, &n, sizeof(n));
	mem += sizeof(n);

	this->info("start seriliazing every node in map");
	for (const auto &item : this->node_map) {
		auto start_address = item.first;

		memcpy(mem, &start_address, sizeof(start_address));
		mem += sizeof(start_address);

		auto node = item.second;
		
		size_t node_size = node->mem_size();
		int n = node->serialize(mem, size);
		if (n != 0) {
			this->error("cannot serialise the node");
			return n;
		}
		mem += node_size;
	}

	memcpy(mem, &this->guard, sizeof(this->guard));
	this->info("serialization is finished, guard value added");

	return 0;
}
