#pragma once

#include "instruction.h"
#include <vector>
#include <string>

class Node {

private:
	std::vector<std::string> block;
	std::vector<size_t> addresses;
	bool is_done = false;
	
	bool it_fits(size_t size) const noexcept;
	bool no_branching() const noexcept;
	std::string graphviz_color() const noexcept;
	std::string graphviz_name() const noexcept;
	std::string graphviz_label() const noexcept;

public:	
	size_t start_address;
	size_t max_occurrences = 1;
	size_t true_branch_address = 0;
	size_t false_branch_address = 0;
	unsigned int occurrences = 1;

	void mark_done() noexcept;
	bool done() const noexcept;
	void append_instruction(Instruction instruction) noexcept;
	bool validate() const noexcept;
	std::string graphviz_definition() const;
	std::string graphviz_relation() const;
	int serialize(uint8_t *mem, const size_t size) const noexcept;
	int deserialize(const uint8_t *mem, const size_t size) noexcept;
	size_t mem_size() const noexcept;
	bool contains_address(size_t eip) const noexcept;
	Node() = default;
	~Node() = default;
};
