#pragma once

#include "instruction.h"
#include <vector>
#include <string>

class Node {

private:
	size_t _start_address = 0;
	std::vector<instruction> block;
	bool is_done = false;
	
	bool it_fits(size_t size) const noexcept;
	bool no_branching() const noexcept;
	std::string graphviz_color() const noexcept;
	std::string graphviz_name() const noexcept;
	std::string graphviz_label() const noexcept;
	void already_visited(size_t eip) noexcept;

public:
	size_t max_occurrences = 1;
	size_t true_branch_address = 0;
	size_t false_branch_address = 0;
	unsigned int occurrences = 1;
    bool is_last_instruction_call() const noexcept;
	void mark_done() noexcept;
    instruction last_api_reporter_instruction() const noexcept;
	size_t start_address() const noexcept;
	bool done() const noexcept;
	void append_instruction(instruction instruction) noexcept;
	void append_branch_instruction(instruction instruction) noexcept;
	std::string graphviz_definition() const;
	std::string graphviz_relation() const;
	int serialize(uint8_t *mem, const size_t size) const noexcept;
	int deserialize(const uint8_t *mem, const size_t size) noexcept;
	size_t mem_size() const noexcept;
	bool contains_address(size_t eip) const noexcept;
	size_t true_neighbour() const noexcept;
	size_t false_neighbour() const noexcept;
	Node(size_t start_address) : _start_address(start_address) {}
	Node() = default;
	~Node() = default;
};
