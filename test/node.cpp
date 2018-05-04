#include <gtest/gtest.h>

#include <cstdio>

#include "src/node.hpp"



TEST(Node, Constructor)
{
	auto node = Node();
}

constexpr const char* def = R"(
	%s [
		label = "%s"
		color = "%s"
	]
)";

static inline std::string generate_definition(
	const char* name, 
	const char* label, 
	const char* color)
{
	char buff[200] = { 0 };
	std::snprintf(buff, 200, def, name, label, color);
	return std::string(buff);
}

static inline std::string generate_relation(
	const size_t start, 
	const size_t end)
{
	char buff[200] = { 0 };
	std::snprintf(buff, 200, "node%lu -> node%lu\\n", start, end);
	return std::string(buff);
}


TEST(Node, graphiz_definition)
{
	auto defintion = generate_definition("node0", "", "1");
	auto expected = defintion.c_str();
	
	auto node = Node();
	auto definitions = node.graphviz_definition();
	auto got = definitions.c_str();
	EXPECT_STREQ(got, expected);

	defintion = generate_definition("node1", "", "1");
	expected = defintion.c_str();

	node.start_address = 1;
	definitions = node.graphviz_definition();
	got = definitions.c_str();
	EXPECT_STREQ(got, expected);

	
	defintion = generate_definition(
		"node1", 
		"push ESP, EBP\\ncmp EZ, 2\\njp 0x2523\\n",
		"1");
	expected = defintion.c_str();

	node.block.push_back("push ESP, EBP");
	node.block.push_back("cmp EZ, 2");
	node.block.push_back("jp 0x2523");
	definitions = node.graphviz_definition();
	got = definitions.c_str();
	EXPECT_STREQ(got, expected);

	node.occurences = 5;
	definitions = node.graphviz_definition();
	got = definitions.c_str();
	EXPECT_STREQ(got, expected);
}

TEST(Node, graphviz_relation)
{
	auto relation = generate_relation(0x0, 0x0);
	relation += relation;
	auto expected = relation.c_str();

	auto node = Node();
	auto relations = node.graphviz_relation();
	auto got = relations.c_str();
	EXPECT_STREQ(got, expected);

	relation = generate_relation(0x5555, 0x6666);
	relation += generate_relation(0x5555, 0x7777);
	expected = relation.c_str();

	node.start_address = 0x5555;
	node.true_branch_address = 0x6666;
	node.false_branch_address = 0x7777;

	relations = node.graphviz_relation();
	got = relations.c_str();
	EXPECT_STREQ(got, expected);
}