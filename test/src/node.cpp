#include <cstdio>
#include <gtest/gtest.h>

#include "src/node.hpp"
#include "src/common.hpp"


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

static inline std::string 
generate_definition(const char* name, const char* label, const char* color)
{
	char buff[200] = { 0 };
	std::snprintf(buff, 200, def, name, label, color);
	return std::string(buff);
}

static inline std::string
generate_relation(const size_t start, const size_t end)
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
	EXPECT_FALSE(definitions.empty());
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
	EXPECT_FALSE(definitions.empty());
	got = definitions.c_str();
	EXPECT_STREQ(got, expected);

	node.occurrences = 5;
	definitions = node.graphviz_definition();
	EXPECT_FALSE(definitions.empty());
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
	EXPECT_FALSE(relations.empty());
	got = relations.c_str();
	EXPECT_STREQ(got, expected);
}

static inline size_t empty_node_size()
{
	return 5 * sizeof(size_t);
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

TEST(Node, serialize_empty)
{
	auto node = Node();
	auto size = node.mem_size();
	auto *mem = new uint8_t[size];
	memset(mem, 0, size);
	const uint8_t expected[] = {
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
	};
	int n = node.serialize(mem, size);
	EXPECT_EQ(n, 0);
	n = memcmp(mem, expected, size);
	EXPECT_EQ(n, 0);
	delete[] mem;
}

TEST(Node, serialize)
{
	auto node = Node();
	node.block.push_back("test");
	node.block.push_back("another test");
	
	auto size = node.mem_size();
	auto *mem = new uint8_t[size];
	memset(mem, 0, size);
	const uint8_t expected[] = {
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00,
		0x74, 0x65, 0x73, 0x74, 0x00,
		0x61, 0x6e, 0x6f, 0x74, 0x68,
		0x65, 0x72, 0x20, 0x74, 0x65,
		0x73, 0x74, 0x00,
	};

	int n = node.serialize(mem, size);
	EXPECT_EQ(n, 0);

	n = memcmp(mem, expected, size);
	EXPECT_EQ(n, 0);
	
	delete[] mem;
}

TEST(Node, deserialize)
{
	const uint8_t expected[] = {
		0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00,
		0x74, 0x65, 0x73, 0x74, 0x00, //test\0
		0x61, 0x6e, 0x6f, 0x74, 0x68, //another test\0
		0x65, 0x72, 0x20, 0x74, 0x65,
		0x73, 0x74, 0x00,
	};
	const size_t size = ARRAY_SIZE(expected);
	
	auto node = Node();
	int err = node.deserialize(expected, size);
	EXPECT_EQ(err, 0);

	EXPECT_EQ(node.start_address, 0);
	EXPECT_EQ(node.occurrences, 1);
	EXPECT_EQ(node.false_branch_address, 0);
	EXPECT_EQ(node.true_branch_address, 0);

	size_t n = node.block.size();
	EXPECT_EQ(n, 2);

	auto str = node.block[0];
	EXPECT_FALSE(str.empty());
	auto got = str.c_str();
	EXPECT_STREQ(got, "test");
	
	str = node.block[1];
	got = str.c_str();
	EXPECT_STREQ(got, "another test");
	
}