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
    
	size_t start = 0; 
    
	size_t next_instr = 0; //TODO: remove this
	
	std::map<size_t, std::shared_ptr<Node>> node_map; 
	
	const uint32_t guard = 0x7777;
	
	std::shared_ptr<Log> logger = nullptr;
	
	bool it_fits(size_t into) const noexcept;
	size_t mem_size() const noexcept;

	void info(const std::string& message) const noexcept;
	void error(const std::string& message) const noexcept;
	void warning(const std::string& message) const noexcept;

public:

    void generate(std::string content, std::string fname = "");

    std::string graphviz(void);

	std::map<size_t, Node*> merge(void);

	int serialize(uint8_t* mem, const size_t size) const noexcept;

	uint8_t* deserialize(uint8_t* mem) noexcept;
	PartialFlowGraph(std::shared_ptr<Log> logger = nullptr) : logger(logger) {}
	~PartialFlowGraph() = default;

	void add(Instruction instr) noexcept;
};