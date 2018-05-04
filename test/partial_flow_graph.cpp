#include <gtest/gtest.h>

#include "src/types.hpp"
#include "src/partial_flow_graph.hpp"

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