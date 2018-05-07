#include <cstdio>
#include <gtest/gtest.h>
#include "src/node.hpp"


TEST(Node, Constructor)
{
	EXPECT_NO_THROW(Node());
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


static inline size_t empty_node_size()
{
	return 5 * sizeof(size_t);
}

static inline uint8_t *empty_node_mem()
{
	auto node = Node();
	size_t need = node.mem_size();
	uint8_t *mem = new uint8_t[need];

	size_t first = 0;
	size_t second = 1;
	memcpy(mem, &first , sizeof(first));
	mem += sizeof(first);
	memcpy(mem, &second, sizeof(second));
	mem += sizeof(second);
	memcpy(mem, &first, sizeof(first));
	mem += sizeof(first);
	memcpy(mem, &first, sizeof(first));
	mem += sizeof(first);
	auto block_size = first;
	memcpy(mem, &block_size, sizeof(block_size));
	mem += sizeof(block_size);
	return mem;
}


TEST(Node, mem_size)
{
	auto expect = empty_node_size();
	auto node = Node();
	auto got = node.mem_size();
	EXPECT_EQ(got, expect);

	node.block.push_back("push ESP");
	node.block.push_back("push EBP");
	expect += strlen("push ESP") + 1;
	expect += strlen("push EBP") + 1;
	got = node.mem_size();
	EXPECT_EQ(got, expect);
}


TEST(Node, serialize_error)
{
	auto node = Node();

	auto err = node.serialize(nullptr, 0);
	EXPECT_EQ(err, EINVAL);

	auto size = node.mem_size();
	auto *mem = new uint8_t[size];
	memset(mem, 0, size);

	err = node.serialize(mem, 0);
	EXPECT_EQ(err, EINVAL);

	for (size_t i = 0; i < size; i++)
		EXPECT_EQ(mem[i], 0);

	err = node.serialize(mem, size - 1);
	EXPECT_EQ(err, ENOMEM);

	for (size_t i = 0; i < size; i++)
		EXPECT_EQ(mem[i], 0);

	delete[] mem;
}


TEST(Node, serialize)
{
	auto node = Node();
	auto size = node.mem_size();
	auto *mem = new uint8_t[size];
	//auto *expected_mem = empty_node_mem();
	memset(mem, 0, size);

	//auto err = node.serialize(mem, size);
	//EXPECT_EQ(err, 0);

	/*err = memcmp(mem, expected_mem, empty_node_size());
	EXPECT_EQ(err, 0);*/

	delete[] mem;
	//delete expected_mem;
}

