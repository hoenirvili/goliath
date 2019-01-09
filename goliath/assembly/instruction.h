#pragma once

#include "goliath/api/types.h"
#include <cstddef>
#include <string>

namespace assembly
{
void patch_next_and_side_addr(CUSTOM_PARAMS *custom_params) noexcept;

/**
 * instruction type holds all context about an instruction
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

    /**
     * create a new instruction based on the context provided
     */
    explicit instruction(size_t eip,
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

    /**
     * load_from_memory starting from the *mem read byte by byte
     * and populate the internal state of the instruction
     */
    void load_from_memory(const std::byte *mem) noexcept;
    /**
     * load_to_memory writes all instruction information staring at
     * the address mem
     */
    void load_to_memory(std::byte *mem) const noexcept;

    // TODO(hoenir): remove this
    bool it_fits(size_t size) const noexcept;

    /**
     * is_ret returns true if the instruction is a RET instruction
     */
    bool is_ret() const noexcept;

    /**
     * validate returns true if the internal instruction
     * contains valid information
     */
    bool validate() const noexcept;

    /**
     * is_call returns true if the instruction is a CALL instruction
     */
    bool is_call() const noexcept;

    /**
     * is_call returns true if the instruction
     * is a branch type instruction
     */
    bool is_branch() const noexcept;

    /**
     * is_leave returns true if the instruction is a LEAVE instruction
     */
    bool is_leave() const noexcept;

    /**
     * direct_branch return true if the instruction is a JUMP type instruction
     */
    bool direct_branch() const noexcept;

    /**
     * str return the string representation of the instruction
     */
    std::string str() const noexcept;

    /**
     * true_branch_address returns the EIP of the next instruction
     * in the execution cycle
     */
    size_t true_branch_address() const noexcept;

    /**
     * false_branch_address returns the EIP of internal side addr
     * of the next instruction in the execution cycle
     */
    size_t false_branch_address() const noexcept;

    /**
     * pointer_address returns the EIP of the instruction
     */
    size_t pointer_address() const noexcept;

    /**
     * mem_size returns how much memory is required in order to
     * store the instruction in bytes
     */
    size_t mem_size() const noexcept;

    instruction() = default;
    ~instruction() = default;
};
}; // namespace assembly
