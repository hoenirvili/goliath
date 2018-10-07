#pragma once

#include "cfgtrace/api/types.h"
#include <string>

/**
 * instruction type holds all context of the next instruction
 */
class instruction
{
private:
    size_t len;            /* instruction length */
    size_t next_node_addr; /* next node address */
    size_t side_node_addr; /* else node address */
    size_t eip;            /* current instruction pointer */
    std::string content;   /* complete instruction */

    const size_t guard_value = 6632; /* magic value */
    bool is_guard_present(const std::byte *mem) const noexcept;

public:
    std::string api_reporter; /* extra information from APIReporter*/
    BRANCH_TYPE branch_type;  /* no branch = 0 */

    instruction(size_t eip,
                char *content,
                BRANCH_TYPE branch_type,
                size_t len,
                size_t next_node_addr,
                size_t side_node_addr) noexcept
        : eip(eip),
          content(content),
          branch_type(branch_type),
          len(len),
          next_node_addr(next_node_addr),
          side_node_addr(side_node_addr)
    {
    }
    void load_from_memory(const std::byte *mem) noexcept;
    void load_to_memory(std::byte *mem) const noexcept;
    bool it_fits(size_t size) const noexcept;
    instruction() = default;
    ~instruction() = default;
    bool is_ret() const noexcept;
    bool validate() const noexcept;
    bool is_call() const noexcept;
    bool is_branch() const noexcept;
    bool is_leave() const noexcept;
    bool direct_branch() const noexcept;
    std::string str() const noexcept;
    size_t true_branch_address() const noexcept;
    size_t false_branch_address() const noexcept;
    size_t pointer_address() const noexcept;
    size_t mem_size() const noexcept;
};
