#include "partial_flow_graph.hpp"
#include "common.hpp"
#include "log.hpp"
#include "types.hpp"

#include <fstream>
#include <assert.h>

using namespace std;

static size_t start_instr;

static vector<NodeMap> storage;

//static Log logger("PartialFlowGraph");

// _merge iterates in *from* and if the element exists in *into*
// increment the occurences fiels or move the element in *into*
static void _merge(const NodeMap from, NodeMap& into)
{
    for (auto& item : from) {
        auto key = item.first;
        auto value = item.second;

        auto it = into.find(key);
        // no key found
        if (it == into.end()) {
            into[key] = value;
            continue;
        }
        // key found
        into[key]->occurences++;
    }
}

// merge merges all NodeMaps from storage and returns it
NodeMap PartialFlowGraph::merge()
{
    assert(storage.empty());
    auto into = storage[0];
    vector<int>::size_type i = 1;
    for (; i != storage.size(); i++)
        _merge(storage[i], into);

    return into;
}

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

PartialFlowGraph::~PartialFlowGraph()
{

    // TODO:
    // HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    // BYTE *shared_mem = (BYTE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    // memset(CFG(shared_mem), 0, 0x8000);
    // memcpy(CFG(shared_mem), formula.c_str(), formula.size());
    // CloseHandle(hMapFile);

    this->nl = this->merge();
    auto graphviz = this->graphviz();
    this->generate(graphviz, "final");

    // clean up
    for (auto& nodelist : storage)
        for (auto& node : nodelist)
            delete node.second;
}

std::string PartialFlowGraph::graphviz()
{
    assert(this->nl.size() != 0);
    assert(this->start != 0);

    std::string definitions = "";
    for (auto& item : this->nl) {
        auto node = item.second;
        definitions += node->graphviz_definition();
    }

    auto digraph = digraph_prefix + definitions;

    for (auto& item : this->nl) {
        auto node = item.second;
        digraph += node->graphviz_realtion();
    }

    return digraph;
}

void PartialFlowGraph::generate(std::string content, std::string fname)
{
    if (content.empty()) {
        return;
    }

    const auto prefix = ".dot";
    if (fname.empty())
        fname = std::to_string(this->start);

    auto file = fstream(fname+prefix);
    file << content;
    file.close();

    const std::string cmd = "dot -Tpng " + fname + prefix + " -o" + fname + ".png";
    auto n = std::system(cmd.c_str());
	//if (!n)
		//logger.error("cannot generate partial flow graph");
}

static inline bool is_branch(unsigned t)
{
    BRANCH_TYPE bt = (BRANCH_TYPE) t;

    switch (bt) {
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

void PartialFlowGraph::add_instruction(Instruction instruction)
{
    if (this->start == instruction.eip) {
        // these fields can't be 0
        assert(this->nl.size() != 0);
        assert(this->start != 0);
        assert(this->next_instr != 0);
        /**
         *after the last node has been added in our
         *partial flow graph, push back the graph
         */
        storage.push_back(this->nl);

        /**
         *generate a control flow graph per every graph in png format
         */
        auto graphviz = this->graphviz();
        this->generate(graphviz);
        /**
         *restart the internal state
         */
        this->nl.clear();    // clear the node list
        this->start = 0;
        this->next_instr = 0;
    }

    Node* node;

    if (!this->start)
        this->start = instruction.eip;

    if (!start_instr)
        start_instr = this->start;

    if (!this->next_instr) {
        this->next_instr = instruction.eip;
        auto n = new Node();
        n->si = this->next_instr;
        this->nl[this->next_instr] = n;
    }

    node = this->nl[this->next_instr];

    /*always test if the current instruction is branching*/
    if (is_branch(instruction.branch_type)) {
        node->tb = instruction.argument_value; /*jump to next node*/
        node->fb = instruction.eip + instruction.len; /*point to the next instruction*/
        this->next_instr = 0; /*we have a new node*/
    }

    node->block.push_back(instruction.content);
}
