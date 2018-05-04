#pragma once

#include <map>

#include "node.hpp"

/**
 * Instruction type holds all context
 * of the next instruction
 */
struct Instruction {
    size_t eip; /*instrction pointer*/
    const char* content; /*complete instruction*/
    unsigned int branch_type; /*no branching = 0*/
    size_t len; /*instruction lenght*/
    size_t argument_value; /*instruction argument*/
};

class PartialFlowGraph {

private:
    size_t start = 0; 
    size_t next_instr = 0;
	std::map<size_t, Node*> node_map; 

    /**
     * generate generates the .dot file in the current directory
     * and runs the dot command on it generating a png file for the graph
     */
    void generate(std::string content, std::string fname = "");

    /**
     * generate the graphviz .dot code from the node list
     */
    std::string graphviz(void);

    /**
     * merge returns the result of merging all partial flow graphs
     */
	std::map<size_t, Node*> merge(void);

public:
    PartialFlowGraph() = default;

    /**
     * Always at the end of the execution put
     * the partial flow graph generated into a storage
     */
    ~PartialFlowGraph();

    /**
     * add_instruction append next instrction in our partial flow graph
     */
    void add_instruction(Instruction instr);
};
