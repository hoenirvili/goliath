#pragma once

#include <map>
#include <memory>
#include <string>

#include "node.hpp"
#include "instruction.hpp"

class PartialFlowGraph {

private:
	
	/**
	 * current_ndoe_addr keeps track of the current node
	 * address that has been added with the add function
	 */
	size_t current_node_addr = 0x0;

	bool ret_instr_encountered = false;
	/**
	 * it_fits returns true if the given size a 
	 * PartialFlowGraph object in memory could fit
	 */
	bool it_fits(const size_t size) const noexcept;
	
public:

	PartialFlowGraph() = default;
	~PartialFlowGraph() = default;
	
	/**
	 * start marks the start of the first address of the first node
	 */
	std::size_t start = 0; 

	/**
	 * node_map internal storage of all nodes in the partial flow graph
	 */
	std::map<std::size_t, std::shared_ptr<Node>> node_map; 
    
	/**
	 * generates the png file of the partial flow graph
	 * given an graphviz script and the file to write it to
	 * in order to execute the dot command line
	 */
	int generate(std::string content, std::ostream* out) const noexcept;

	/**
	 * mem_size returns the the minimum size of memory that
	 * is needed to write the hole binary partial flow graph structure
	 */
	size_t mem_size() const noexcept;

	/**
	 * graphviz returns the graphviz script that is generated
	 * based on the current partial flow graph state
	 */
	std::string graphviz() const;

	/**
	 * merge the partial flow graph given with the current one
	 * in case if the from does not match the start address it returns
	 * EINVAL
	 */
	int merge(const PartialFlowGraph& from) noexcept;

	/**
	 * serialize writes the partial flow graph binary structure
	 * into mem without exceeding the size specified
	 */
	int serialize(uint8_t* mem, size_t size) const noexcept;
	
	/**
	 * deserialize reads write the partial flow graph binary structure
	 * into mem without exceeding the size specified
	 */
	int deserialize(const uint8_t* mem, size_t size) noexcept;

	/**
	 * add adds a new instruction into the corresponding partial
	 * flow graphs nodes
	 * if the operation fails or the instruction is not valid 
	 * it returns EINVAL
	 */
	int add(const Instruction& instr) noexcept;
};