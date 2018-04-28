#pragma once

#include <vector>

/**
 * Node type is a single node in
 * the partial flow graph
 */
typedef struct Node {
    size_t si; /*start eip instruction*/
    std::vector<std::string> block; /*all instructions of node*/
    size_t tb; /*true branch*/
    size_t fb; /*false branch*/
    unsigned int occurences = 1; /*at least one occurence*/

public:
    /*graphviz_definition returns the node in graphviz definition format*/
    std::string graphviz_definition(void);
    /*graphviz_realtion return the raltion of the node in graphviz format */
    std::string graphviz_realtion(void);
} Node;
