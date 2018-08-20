#pragma once

#include "cfgtrace/instruction.h"
#include "node.h"
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
    void append_node_neighbours(const std::unique_ptr<Node> &node) noexcept;
    bool it_fits(const size_t size) const noexcept;
    size_t set_and_get_current_address(size_t eip) noexcept;
    std::unique_ptr<Node> get_current_node(size_t start_address) noexcept;

public:
    size_t start_address_first_node = 0;
    std::map<size_t, std::unique_ptr<Node>> nodes;

    void generate(std::string content, std::ostream *out) const;
    std::string graphviz();
    void serialize(uint8_t *mem, size_t size) const;
    void deserialize(const uint8_t *mem, size_t size);
    size_t mem_size() const noexcept;
    bool node_exists(size_t start) const noexcept;
    void append_instruction(instruction instruction);
    void append_branch_instruction(instruction instruction);
    control_flow_graph() = default;
    ~control_flow_graph() = default;
};
