#pragma once

#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/graph/node.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace graph
{
class control_flow
{
private:
    using node_ptr = std::unique_ptr<Node>;

    size_t current_node_start_addr = 0x0;
    size_t current_pointer = 0x0;

    bool node_contains_address(size_t address) const noexcept;
    void set_nodes_max_occurrences() noexcept;
    void unset_current_address(const node_ptr &node) noexcept;
    void append_node_neighbors(const node_ptr &node) noexcept;
    bool it_fits(const size_t size) const noexcept;
    size_t set_and_get_current_address(size_t eip) noexcept;
    node_ptr get_current_node(size_t start_address) noexcept;

public:
    size_t start_address_first_node = 0;
    std::map<size_t, node_ptr> nodes;

    control_flow() = default;
    ~control_flow() = default;
    void generate(std::string content, std::ostream *out, int it) const;
    std::string graphviz();
    void load_to_memory(std::byte *mem) const noexcept;
    void load_from_memory(const std::byte *mem) noexcept;
    size_t mem_size() const noexcept;
    bool node_exists(size_t start) const noexcept;
    void append_instruction(assembly::instruction instruction);
    void append_branch_instruction(assembly::instruction instruction);
};

using creator = std::function<control_flow *()>;

bool is_initialised() noexcept;
control_flow *instance() noexcept;
void clean() noexcept;
void custom_creation(creator create) noexcept;

}; // namespace graph
