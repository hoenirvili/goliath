#pragma once

#include <map>

#include "types.hpp"
#include "node.hpp"
#include "log.hpp"


/**
 * Instruction type holds all context
 * of the next instruction
 */
class Instruction {

public:
	size_t eip;					/* instruction pointer */
    const char* content;		/* complete instruction */
    Int32 branch_type;			/* no branch = 0 */
    size_t len;					/* instruction length */
    size_t argument_value;		/* instruction branch argument */

	/**
	 * validate returns true if the 
	 * instruction object contains valid value
	 * fields 
	 */
	bool validate() const;

	/**
	 * is_branch returns true if the
	 * instruction is a instruction that can branch
	 */
	bool is_branch() const noexcept;

	/**
	 * true_branch returns the address that
	 * the branch will jump if it evaluates to true
	 */
	size_t true_branch() const noexcept;

	/**
	 * false_branch returns the address that
	 * the branch will jump if it evaluates to false
	 */
	size_t false_branch() const noexcept;
};

class PartialFlowGraph {

private:
	
	/**
	 * should_alloc_node is true the add function will alloc
	 * a new node and push it into node_map
	 */
	bool should_alloc_node = true;

	/**
	 * current_ndoe_addr maintains the current node
	 * address that has been added with the add function
	 */
	size_t current_node_addr = 0x0;

	/**
	 * guard is a special guard value used in finishing
	 * the serialization process
	 */
	const uint16_t guard = 0x7777;

	/**
	 * logger internal logger to output various message
	 */
	std::shared_ptr<Log> logger = nullptr;
	
	/**
	 * it_fits returns true if the given size a 
	 * PartialFlowGraph object in memory could fit
	 */
	bool it_fits(const size_t size) const noexcept;
	
	/**
	 * info outputs a info message using the internal logger
	 */
	void info(const std::string& message) const noexcept;

	/**
	 * error outputs a error message using the internal logger
	 */
	void error(const std::string& message) const noexcept;

	/**
	 * warning outputs a warning message using the internal logger
	 */
	void warning(const std::string& message) const noexcept;

public:

	PartialFlowGraph(
		std::shared_ptr<Log> logger = nullptr
	) : logger(logger) {}
	
	~PartialFlowGraph() = default;
	
	/**
	 * start marks the start of the first address of the first node
	 */
	size_t start = 0; 

	/**
	 * node_map internal storage of all nodes in the partial flow graph
	 */
	std::map<size_t, std::shared_ptr<Node>> node_map; 
    
	/**
	 * generates the png file of the partial flow graph
	 * given an graphviz script and the file to write it to
	 * in order to execute the dot command line
	 */
	int generate(std::string content, std::ostream *out) const noexcept;

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
	int merge(const PartialFlowGraph &from) noexcept;

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
	int add(Instruction instr) noexcept;
};