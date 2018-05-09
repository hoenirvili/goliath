#include <gtest/gtest.h>
#include "src/types.hpp"
#include "src/partial_flow_graph.hpp"
#include "src/log.hpp"
#include "src/common.hpp"

using namespace std;

TEST(Instruction, is_branching)
{
	Instruction instruction = {0};
	
	bool got = instruction.is_branch();
	EXPECT_FALSE(got);

	instruction.branch_type = JP;
	got = instruction.is_branch();
	EXPECT_TRUE(got);
}

TEST(Instruction, true_branch)
{
	Instruction instruction = { 0 };

	auto branch = instruction.true_branch();
	EXPECT_EQ(branch, 0x0);

	instruction.argument_value = 0x6000;
	branch = instruction.true_branch();
	EXPECT_EQ(branch, 0x6000);
}

TEST(Instruction, false_branch)
{
	Instruction instruction = { 0 };

	auto branch = instruction.false_branch();
	EXPECT_EQ(branch, 0x0);

	instruction.eip = 0x6000;
	branch = instruction.false_branch();
	EXPECT_EQ(branch, 0x6000);

	instruction.len = 0x4;
	branch = instruction.false_branch();
	EXPECT_EQ(branch, 0x6004);
}

TEST(Instruction, validate_empty)
{
	Instruction instruction = { 0 };

	auto got = instruction.validate();
	EXPECT_FALSE(got);

}

TEST(Instruction, validate_with_errors)
{
	Instruction instruction;
	instruction.content = "content";

	auto got = instruction.validate();
	EXPECT_FALSE(got);

	instruction.branch_type = -1;
	got = instruction.validate();
	EXPECT_FALSE(got);
}

TEST(Instruction, validate)
{
	Instruction instruction;
	instruction.content = "content";
	instruction.branch_type = JC;
	
	auto got = instruction.validate();
	EXPECT_TRUE(got);
}

TEST(PartialFlowGraph, Constructor)
{
	
	EXPECT_NO_THROW(PartialFlowGraph(nullptr));
	EXPECT_NO_THROW(PartialFlowGraph());
	auto logger = make_shared<Log>();
	EXPECT_NO_THROW(PartialFlowGraph(logger));
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
	auto pfg = PartialFlowGraph(nullptr);
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
		EXPECT_EQ(node->occurences, 1);
		EXPECT_EQ(node->false_branch_address, 0);
		EXPECT_EQ(node->true_branch_address, 0);

		n = node->block.size();
		EXPECT_EQ(n, 2);

		auto str = node->block[0];
		auto got = str.c_str();
		EXPECT_STREQ(got, "test");

		str = node->block[1];
		got = str.c_str();
		EXPECT_STREQ(got, "another test");
	}
}