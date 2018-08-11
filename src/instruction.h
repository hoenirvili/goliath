#pragma once

#include "api.h"
#include <string>

/**
 * instruction type holds all context of the next instruction
 */
class instruction
{
private:
    const size_t len;            /* instruction length */
    const size_t next_node_addr; /* next node address */
    const size_t side_node_addr;
    const size_t eip;          /* current instruction pointer */
    const std::string content; /* complete instruction */

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

    instruction() = delete;
    ~instruction() = default;
    bool is_ret() const noexcept;
    bool validate() const noexcept;
    bool is_call() const noexcept;
    bool is_branch() const noexcept;
    bool direct_branch() const noexcept;
    std::string string() const noexcept;
    size_t true_branch_address() const noexcept;
    size_t false_branch_address() const noexcept;
    size_t pointer_address() const noexcept;
};
