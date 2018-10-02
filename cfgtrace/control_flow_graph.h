#pragma once

#include "cfgtrace/instruction.h"
#include "cfgtrace/node.h"
#include <map>
#include <memory>
#include <string>

class control_flow_graph
{
private:
    size_t current_node_start_addr = 0x0;
    size_t current_pointer = 0x0;

    bool node_contains_address(size_t address) const noexcept;
    void set_nodes_max_occurrences() noexcept;
    void unset_current_address(const std::unique_ptr<Node> &node) noexcept;
    void append_node_neighbors(const std::unique_ptr<Node> &node) noexcept;
    bool it_fits(const size_t size) const noexcept;
    size_t set_and_get_current_address(size_t eip) noexcept;
    std::unique_ptr<Node> get_current_node(size_t start_address) noexcept;

public:
    size_t start_address_first_node = 0;

    std::map<size_t, std::unique_ptr<Node>> nodes;

    control_flow_graph() = default;
    ~control_flow_graph() = default;
    void generate(std::string content, std::ostream *out, int it) const;
    std::string graphviz();
    void load_to_memory(uint8_t *mem) const noexcept;
    void load_from_memory(const uint8_t *mem) noexcept;
    size_t mem_size() const noexcept;
    bool node_exists(size_t start) const noexcept;
    void append_instruction(instruction instruction);
    void append_branch_instruction(instruction instruction);
};
