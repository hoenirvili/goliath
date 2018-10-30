#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/error/error.h"
#include "cfgtrace/logger/logger.h"
#include <stdexcept>
#include <string>

namespace assembly
{
static inline bool _direct_branch(BRANCH_TYPE type) noexcept
{
    return type == JmpType;
}

static inline bool _is_ret(BRANCH_TYPE type) noexcept
{
    return type == RetType;
}

static inline bool _is_call(BRANCH_TYPE type) noexcept
{
    return type == CallType;
}

static inline bool _is_branch(BRANCH_TYPE type) noexcept
{
    switch (type) {
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

// TODO(hoenir): the engine should fix this
static size_t compute_side_addr(CUSTOM_PARAMS *custom_params)
{
    BRANCH_TYPE type = (BRANCH_TYPE)custom_params->MyDisasm->Instruction.BranchType;

    if (_direct_branch(type))
        return 0;
    if (_is_ret(type))
        return 0;

    size_t eip = custom_params->MyDisasm->EIP;
    size_t len = custom_params->instrlen;

    size_t false_branch = eip + len;
    if (_is_call(type)) {
        if (custom_params->next_addr == custom_params->side_addr && custom_params->next_addr == false_branch)
            return 0;

        return false_branch;
    }
    if (!_is_call(type) && !_is_ret(type))
        return custom_params->side_addr;

    return false_branch;
}

static size_t compute_next_addr(CUSTOM_PARAMS *custom_params)
{
    return custom_params->next_addr;
}

void patch_next_and_side_addr(CUSTOM_PARAMS *custom_params) noexcept
{
    custom_params->next_addr = compute_next_addr(custom_params);
    custom_params->side_addr = compute_side_addr(custom_params);
}

bool instruction::is_branch() const noexcept
{
    return _is_branch(this->branch_type);
}

bool instruction::is_leave() const noexcept
{
    return this->content == "leave ";
}

size_t instruction::mem_size() const noexcept
{
    size_t sz = 0;
    sz += sizeof(this->eip);
    if (this->api_reporter.empty())
        sz += sizeof(this->guard_value);
    else
        sz += this->api_reporter.size() + 1; // with the \0
    if (this->content.empty())
        sz += sizeof(this->guard_value);
    else
        sz += this->content.size() + 1; // with the \0
    sz += sizeof(this->branch_type);
    sz += sizeof(this->len);
    sz += sizeof(this->next_node_addr);
    sz += sizeof(this->side_node_addr);
    return sz;
}

bool instruction::it_fits(size_t size) const noexcept
{
    return (size >= this->mem_size());
}

bool instruction::is_guard_present(const std::byte *mem) const noexcept
{
    return (memcmp(&this->guard_value, mem, sizeof(this->guard_value)) == 0);
}

void instruction::load_from_memory(const std::byte *mem) noexcept
{
    memcpy(&this->len, mem, sizeof(this->len));
    mem += sizeof(this->len);

    memcpy(&this->next_node_addr, mem, sizeof(this->next_node_addr));
    mem += sizeof(this->next_node_addr);

    memcpy(&this->side_node_addr, mem, sizeof(this->side_node_addr));
    mem += sizeof(this->side_node_addr);

    memcpy(&this->eip, mem, sizeof(this->eip));
    mem += sizeof(this->eip);

    if (!this->is_guard_present(mem)) {
        // content
        const char *cmem = reinterpret_cast<const char *>(mem);
        size_t cmem_size = strlen(cmem);
        this->content = std::string(cmem, cmem_size);
        mem += cmem_size + 1; // skip also the \0
    } else
        mem += sizeof(this->guard_value);

    if (!this->is_guard_present(mem)) {
        // api_reporter
        const char *cmem = reinterpret_cast<const char *>(mem);
        size_t cmem_size = strlen(cmem);
        this->api_reporter = std::string(cmem, cmem_size);
        mem += cmem_size + 1; // skip also the \0
    } else
        mem += sizeof(this->guard_value);

    memcpy(&this->branch_type, mem, sizeof(this->branch_type));
    mem += sizeof(this->branch_type); // TODO(hoenir): I think this should be removed
}

void instruction::load_to_memory(std::byte *mem) const noexcept
{
    memcpy(mem, &this->len, sizeof(this->len));
    mem += sizeof(this->len);

    memcpy(mem, &this->next_node_addr, sizeof(this->next_node_addr));
    mem += sizeof(this->next_node_addr);

    memcpy(mem, &this->side_node_addr, sizeof(this->side_node_addr));
    mem += sizeof(this->side_node_addr);

    memcpy(mem, &this->eip, sizeof(this->eip));
    mem += sizeof(this->eip);

    // content if any
    if (!this->content.empty()) {
        const size_t cmem_size = this->content.size() + 1; // add also the \0 in mem
        memcpy(mem, this->content.c_str(), cmem_size);
        mem += cmem_size;
    } else {
        memcpy(mem, &this->guard_value, sizeof(this->guard_value));
        mem += sizeof(guard_value);
    }

    // api_reporter if any
    if (!this->api_reporter.empty()) {
        const size_t rep_size = this->api_reporter.size() + 1; // add also the \0 in mem
        memcpy(mem, this->api_reporter.c_str(), rep_size);
        mem += rep_size;
    } else {
        memcpy(mem, &this->guard_value, sizeof(this->guard_value));
        mem += sizeof(guard_value);
    }

    memcpy(mem, &this->branch_type, sizeof(this->branch_type));
    mem += sizeof(this->branch_type); // TODO(hoenir): I think this should be removed
}

std::string instruction::str() const noexcept
{
    if (this->api_reporter.empty())
        return this->content;

    return this->content + " " + this->api_reporter;
}

size_t instruction::true_branch_address() const noexcept
{
    if (!this->api_reporter.empty())
        return 0;

    return this->next_node_addr;
}

size_t instruction::false_branch_address() const noexcept
{
    return this->side_node_addr;
}

bool instruction::is_call() const noexcept
{
    return _is_call(this->branch_type);
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
    return _is_ret(this->branch_type);
}

bool instruction::validate() const noexcept
{
    if (this->content.empty()) {
        logger_warning("empty instruction content");
        return false;
    }

    if (!this->is_branch() && this->branch_type != 0) {
        logger_warning("invalid branch passed %d", this->branch_type);
        return false;
    }

    if (this->eip == 0) {
        logger_warning("invalid eip instruction %d", this->eip);
        return false;
    }

    if (this->len == 0) {
        logger_warning("invalid instruction len : %d", this->len);
        return false;
    }

    if (this->next_node_addr == 0) {
        logger_warning("invalid next node address : %d", this->next_node_addr);
        return false;
    }

    return true;
}

}; // namespace assembly
