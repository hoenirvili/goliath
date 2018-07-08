#pragma once

#include <map>
#include <memory>
#include <string>

#include "node.h"
#include "instruction.h"

class ControlFlowGraph {

private:
	size_t current_node_start_addr = 0x0;
	size_t current_pointer = 0x0;
	bool first = false;

	bool node_contains_address(size_t address) const noexcept;
	void set_nodes_max_occurrences() noexcept;
	bool it_fits(const size_t size) const noexcept;

public:
	size_t start_address_first_node = 0;
	std::map<size_t, std::unique_ptr<Node>> nodes;
	
	int generate(std::string content, std::ostream* out) const noexcept;
	std::string graphviz();
	int serialize(uint8_t* mem, size_t size) const noexcept;
	int deserialize(const uint8_t* mem, size_t size) noexcept;
	size_t mem_size() const noexcept;
	bool node_exists(size_t address) const noexcept;
	int append_instruction(Instruction instruction) noexcept;
	
	ControlFlowGraph() = default;
	~ControlFlowGraph() = default;
};