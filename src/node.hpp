#pragma once

#include <vector>
#include <string>

/**
 * Node object represents a single node in a
 * partial flow graph
 */
class Node {

private:
	/**
	 * is_done is true when all the internal
	 * node fiels are set
	 */
	bool is_done = false;

	/**
	 * it_fits returns true if the size specified is greater or
	 * equal than the object node members
	 */
	bool it_fits(size_t size) const noexcept;

	/**
	* no_branching returns true if node does not branch
	*/
	bool no_branching() const noexcept;

	/**
	* graphviz_color returns graphviz color definitions for the node
	*/
	std::string graphviz_color() const noexcept;

	/**
	 * graphviz_name returns the graphviz node name definition
	 */
	std::string graphviz_name() const noexcept;

	/**
	 * graphviz_label returns the graphviz label definitions
	 */
	std::string graphviz_label() const noexcept;

public:
	
	size_t max_occurrences = 1;

	/**/
	void mark_done() noexcept;

	bool done() const noexcept;

	bool last_instruction_ret = false;
	
	/**
	 * validate returns true if the node contains
	 * valid node information
	 */
	bool validate() const noexcept;

	/**
	* start_address is the first
	* start instruction assembly block code
	*/
	size_t start_address = 0;
    
	/**
	* block holds a list of 
	* every assembly block instruction in node
	*/
	std::vector<std::string> block;
    
	/**
	* true_branch_address stores the next address if the
	* assembly branch evaluates to true
	*/
	size_t true_branch_address = 0;
	
	/**
	* false_branch_address stores the next address if the
	* assembly branch evaluates to false
	*/
	size_t false_branch_address = 0;
    
	/**
	* occurrences the number of times a execution
	* had passed trough this node
	*/
	unsigned int occurrences = 1;
	
	/**
	* graphviz_definition the definitions 
	* of the node in graphviz format
	*/
	std::string graphviz_definition() const;
	
	/**
	* graphviz_relation return the node relation
	*/
	std::string graphviz_relation() const;

	/**
	* serialize all contents of the node into the specified mem location
	* in order to write in that location size must be huge enough
	*/
	int serialize(uint8_t* mem, const size_t size) const noexcept;
	
	/**
	* deserialize extract byte per byte overwriting all internal state
	* until we consume all size bytes
	*/
	int deserialize(const uint8_t* mem, const size_t size) noexcept;
	
	/**
	* mem_size returns the number of bytes that's needed for serialization
	*/
	size_t mem_size() const noexcept;
};
