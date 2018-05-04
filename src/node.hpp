#pragma once

#include <vector>

/**
 * Node object represents a single node in a
 * partial flow graph
 */
class Node {

public:
	/**
	* start_address is the first
	* start instuction assembly block code
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
	* occurences the number of times a execution
	* had passed trough this node
	*/
	unsigned int occurences = 1;
	
	/**
	* graphviz_definition the definitions 
	* of the node in graphviz format
	*/
	std::string graphviz_definition() const;
	
	/**
	* graphviz_relation return the node relation
	*/
	std::string graphviz_relation() const;
};


