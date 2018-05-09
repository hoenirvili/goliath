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
	size_t eip;					/* instrction pointer */
    const char* content;		/* complete instruction */
    Int32 branch_type;			/* no branch = 0 */
    size_t len;					/* instruction lenght */
    size_t argument_value;		/* instruction argument */

	bool validate() const;
	bool is_branch() const noexcept;
	size_t true_branch() const noexcept;
	size_t false_branch() const noexcept;
};

class PartialFlowGraph {

private:
	bool should_alloc_node = true;
	size_t current_node_addr = 0x0;
	const uint16_t guard = 0x7777;
	std::shared_ptr<Log> logger = nullptr;
	
	bool it_fits(const size_t size) const noexcept;
	void info(const std::string& message) const noexcept;
	void error(const std::string& message) const noexcept;
	void warning(const std::string& message) const noexcept;

public:
	size_t start = 0; 
	std::map<size_t, std::shared_ptr<Node>> node_map; 

	PartialFlowGraph(std::shared_ptr<Log> logger = nullptr) : logger(logger) {}
	~PartialFlowGraph() = default;
    
	int generate(std::string content, std::ostream *out) const noexcept;

	size_t mem_size() const noexcept;
	std::string graphviz() const;
	int merge(const PartialFlowGraph &from) noexcept;

	int serialize(uint8_t* mem, size_t size) const noexcept;
	int deserialize(const uint8_t* mem, size_t size) noexcept;
	int add(Instruction instr) noexcept;
};