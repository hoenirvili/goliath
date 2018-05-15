#include <gtest/gtest.h>

#include "src/types.hpp"
#include "src/partial_flow_graph.hpp"
#include "src/log.hpp"
#include "src/common.hpp"

using namespace std;

TEST(PartialFlowGraph, constructor)
{
	EXPECT_NO_THROW(PartialFlowGraph());
}

TEST(PartialFlowGraph, add_with_errors)
{
	auto pfg = PartialFlowGraph();
	Instruction instruction = { 0 };
	int err = pfg.add(instruction);
	EXPECT_EQ(err, EINVAL);
}

#define default_instructions()					\
	{											\
		{0x5555, "push ebp", 0, 4, 0},			\
		{ 0x5559, "mov ebp, esp", 0, 4, 0 },	\
		{ 0x55bb, "push 20", 0, 4, 0 },			\
		{ 0x15bf, "call 0x63215", CallType, 2, 0 },	\
		{ 0x4125, "add esp 4", 0, 2, 0 },		\
		{ 0x2141, "mov DWORD PTR s[ebp], eax", 0,2,0 },\
	}

TEST(PartialFlowGraph, add)
{
	auto pfg = PartialFlowGraph();
	Instruction instructions[] = default_instructions();

	size_t n = ARRAY_SIZE(instructions);
	int err = 0;
	for (size_t i = 0; i < n; i++) {
		err = pfg.add(instructions[0]);
		EXPECT_EQ(err, 0);
	}
}

TEST(PartialFlowGraph, serialize_empty)
{
	size_t max = BUFFER_SIZE - SHARED_CFG;
	auto pfg = PartialFlowGraph();
	size_t size = pfg.mem_size();

	auto *mem = new uint8_t[size];
	memset(mem, 0, sizeof(size));

	uint8_t expected[] = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x77, 0x77
	};

	int n = pfg.serialize(mem, size);
	EXPECT_EQ(n, 0);

	n = memcmp(mem, expected, size);
	EXPECT_EQ(n, 0);

	delete mem;
}


TEST(PartialFlowGraph, serialize_with_errors)
{
	auto pfg = PartialFlowGraph();
	int n = pfg.serialize(nullptr, 0);
	EXPECT_EQ(n, EINVAL);
	
	auto *mem = new uint8_t[10];
	n = pfg.serialize(mem, 0);
	EXPECT_EQ(n, EINVAL);
	
	n = pfg.serialize(mem, 5);
	EXPECT_EQ(n, ENOMEM);
	
	delete[] mem;
}

TEST(PartialFlowGraph, serialize)
{
	auto pfg = PartialFlowGraph();
	
	Instruction instructions[] = default_instructions();
	size_t n = ARRAY_SIZE(instructions);
	int err = 0;

	for (size_t i = 0; i < n; i++) {
		err = pfg.add(instructions[i]);
		EXPECT_EQ(err, 0);
	}

	auto size = pfg.mem_size();
	auto *mem = new uint8_t[size];
	
	err = pfg.serialize(mem, size);
	EXPECT_EQ(err, 0);

	delete[]mem;
}

TEST(PartialFlowGraph, deserialize_empty)
{
	uint8_t expected[] = {
		0x00, 0x00, 0x00, 0x00, /*start*/
		0x00, 0x00, 0x00, 0x00, /*node_map.size*/
		0x77, 0x77 /*guard*/
	};
	size_t size = ARRAY_SIZE(expected);
	
	auto pfg = PartialFlowGraph();
	
	int err = pfg.deserialize(expected, size);
	EXPECT_EQ(err, 0);

	EXPECT_EQ(pfg.start, 0);
	int n = pfg.node_map.size();
	EXPECT_EQ(n, 0);
}

TEST(PartialFlowGraph, deserialize)
{
	uint8_t expected[] = {
		0x05, 0x00, 0x00, 0x00, /*start*/
		0x02, 0x00, 0x00, 0x00, /*node_map.size*/
		0x55, 0x44, 0x33, 0x22,/*key*/
		/*node*/
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00,
		0x74, 0x65, 0x73, 0x74, 0x00, //test\0
		0x61, 0x6e, 0x6f, 0x74, 0x68, //another test\0
		0x65, 0x72, 0x20, 0x74, 0x65,
		0x73, 0x74, 0x00,
		0x99, 0x88, 0x77, 0x66, /*key*/
		/*node*/
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00,
		0x74, 0x65, 0x73, 0x74, 0x00, //test\0
		0x61, 0x6e, 0x6f, 0x74, 0x68, //another test\0
		0x65, 0x72, 0x20, 0x74, 0x65,
		0x73, 0x74, 0x00,
		0x77, 0x77 /*guard*/
	};
	size_t size = ARRAY_SIZE(expected);
	auto pfg = PartialFlowGraph();
	
	int err = pfg.deserialize(expected, size);
	EXPECT_EQ(err, 0);

	EXPECT_EQ(pfg.start, 5);
	int n = pfg.node_map.size();
	EXPECT_EQ(n, 2);
	
	size_t nodes[] = { 0x22334455, 0x66778899 };
	for (size_t i = 0; i < ARRAY_SIZE(nodes); i++) {
		auto key = nodes[i];
		EXPECT_NO_THROW(pfg.node_map.at(key));

		auto node = pfg.node_map[key];
		EXPECT_TRUE(node);

		n = node->block.size();
		EXPECT_EQ(node->start_address, 0);
		EXPECT_EQ(node->occurrences, 1);
		EXPECT_EQ(node->false_branch_address, 0);
		EXPECT_EQ(node->true_branch_address, 0);

		n = node->block.size();
		EXPECT_EQ(n, 2);

		auto str = node->block[0];
		EXPECT_FALSE(str.empty());
		auto got = str.c_str();
		EXPECT_STREQ(got, "test");

		str = node->block[1];
		EXPECT_FALSE(str.empty());
		got = str.c_str();
		EXPECT_STREQ(got, "another test");
	}
}

TEST(PartialFlowGraph, merge_with_errors)
{
	auto pfg = PartialFlowGraph();
	auto from = PartialFlowGraph();
	
	int err = pfg.merge(from);
	EXPECT_EQ(err, EINVAL);

	from.start = 0x7231;

	err = pfg.merge(from);
	EXPECT_EQ(err, EINVAL);
}

TEST(PartialFlowGraph, merge)
{
	auto pfg = PartialFlowGraph();
	auto from = PartialFlowGraph();
	auto node1 = make_shared<Node>();
	node1->start_address = 0x7777;
	node1->occurrences = 1;
	auto node2 = make_shared<Node>();
	node2->occurrences = 1;
	node2->start_address = 0x6666;
	auto node3 = make_shared<Node>();
	node3->occurrences = 1;
	node3->start_address = 0x7777;

	pfg.node_map[0x15123] = node3;
	from.node_map[0x15123] = node1;
	from.node_map[0x52341] = node2;

	int err = pfg.merge(from);
	EXPECT_EQ(err, 0);
	
	EXPECT_EQ(pfg.node_map[0x15123]->occurrences, 2);
	EXPECT_EQ(pfg.node_map[0x15123]->start_address, 0x7777);
	EXPECT_EQ(pfg.node_map[0x52341]->occurrences, 1);
	EXPECT_EQ(pfg.node_map[0x52341]->start_address, 0x6666);
}

constexpr const char* digraph_prefix_expected = R"(
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

constexpr const char* graphviz_expected = R"(
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

	node135954 [
		label = "SUB EBS, 20\n"
		color = "1"
	]

	node201299 [
		label = "PUSH ESP\nPUSH EBP\nSUB EBS, 20\nJP 0x21312\n"
		color = "1"
	]
node135954 -> node139810\nnode135954 -> node12578\nnode201299 -> node135954\nnode201299 -> node4338706\n)";


TEST(PartialFlowGraph, graphviz_empty)
{
	auto pfg = PartialFlowGraph();
	auto definitions = pfg.graphviz();
	EXPECT_FALSE(definitions.empty());
	auto str = definitions.c_str();
	EXPECT_STREQ(str, digraph_prefix_expected);
}

TEST(PartialFlowGraph, graphviz)
{
	auto pfg = PartialFlowGraph();
	auto node = make_shared<Node>();
	node->start_address = 0x31253;
	node->block.push_back("PUSH ESP");
	node->block.push_back("PUSH EBP");
	node->block.push_back("SUB EBS, 20");
	node->block.push_back("JP 0x21312");
	node->false_branch_address = 0x423412;
	node->true_branch_address = 0x21312;
	pfg.node_map[0x31253] = node;

	node = make_shared<Node>();
	node->start_address = 0x21312;
	node->block.push_back("SUB EBS, 20");
	node->false_branch_address = 0x03122;
	node->true_branch_address = 0x22222;
	pfg.node_map[0x21312] = node;

	auto definitions = pfg.graphviz();
	EXPECT_FALSE(definitions.empty());
	auto str = definitions.c_str();
	EXPECT_FALSE(definitions.empty());
	EXPECT_STREQ(str, graphviz_expected);
}