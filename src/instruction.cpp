#include "api.h"
#include "instruction.h"
#include "log.h"

bool instruction::is_branch() const noexcept
{
    switch (this->branch_type) {
    case JO:
    case JC:
    case JE:
    case JA:
    case JS:
    case JP:
    case JL:
    case JG:
    case JB:
    case JECXZ:
    case JmpType:
    case CallType:
    case RetType:
    case JNO:
    case JNC:
    case JNE:
    case JNA:
    case JNS:
    case JNP:
    case JNL:
    case JNG:
    case JNB:
        return true;
    }

    return false;
}

std::string instruction::string() const noexcept
{
    return this->content;
}

size_t instruction::true_branch_address() const noexcept
{
    // we are dealing with an relative address
    if (this->next_node_addr == 0xFFFFFFFF)
        return *(uintptr_t *)(this->side_node_addr);

    return this->next_node_addr;
}

size_t instruction::false_branch_address() const noexcept
{
    if (this->direct_branch())
        return 0;
    if (this->is_ret())
        return 0;

    // try to avoid nasty engine bug
    if (this->is_call())
        return this->eip + this->len;

    if (!this->is_ret() && !this->is_call())
        return this->side_node_addr;

    return this->eip + this->len;
}

bool instruction::is_call() const noexcept
{
    return (this->branch_type == CallType);
}

bool instruction::direct_branch() const noexcept
{
    return (this->branch_type == JmpType);
}

size_t instruction::pointer_address() const noexcept
{
    return this->eip;
}

bool instruction::is_ret() const noexcept
{
    return (this->branch_type == RetType);
}

bool instruction::validate() const noexcept
{
    if (this->content.empty()) {
        log_warning("empty instruction content");
        return false;
    }

    if (!this->is_branch() && this->branch_type != 0) {
        log_warning("invalid branch passed %d", this->branch_type);
        return false;
    }

    if (this->eip == 0) {
        log_warning("invalid eip instruction %d", this->eip);
        return false;
    }

    if (this->len == 0) {
        log_warning("invalid instruction len : %d", this->len);
        return false;
    }

    if (this->next_node_addr == 0) {
        log_warning("invalid next node address : %d", this->next_node_addr);
        return false;
    }

    return true;
}
