#include "cfgtrace/instruction.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/logger/logger.h"
#include <stdexcept>
#include <string>

using namespace std;

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

size_t instruction::mem_size() const noexcept
{
    size_t sz = 0;
    sz += sizeof(this->eip);
    sz += this->api_reporter.size();
    sz += this->content.size();
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

void instruction::deserialize(const uint8_t *mem, const size_t size)
{
    if (!it_fits(size))
        throw invalid_argument("size provided does not fit");

    memcpy(&this->len, mem, sizeof(this->len));
    mem += sizeof(this->len);

    memcpy(&this->next_node_addr, mem, sizeof(this->next_node_addr));
    mem += sizeof(this->next_node_addr);

    memcpy(&this->side_node_addr, mem, sizeof(this->side_node_addr));
    mem += sizeof(this->side_node_addr);

    memcpy(&this->eip, mem, sizeof(this->eip));
    mem += sizeof(this->eip);

    // content
    char *cmem = (char *)mem;
    size_t cmem_size = strlen(cmem);
    this->content = string(cmem, cmem_size);
    mem += cmem_size + 1; // skip also the \0

    // api_reporter
    cmem = (char *)mem;
    cmem_size = strlen(cmem);
    this->api_reporter = string(cmem, cmem_size);
    mem += cmem_size + 1; // skip also the \0

    memcpy(&this->branch_type, mem, sizeof(this->branch_type));
    mem += sizeof(this->branch_type);
}

void instruction::serialize(uint8_t *mem, const size_t size) const
{
    if (!it_fits(size))
        throw invalid_argument("size provided does not fit");

    memcpy(mem, &this->len, sizeof(this->len));
    mem += sizeof(this->len);

    memcpy(mem, &this->next_node_addr, sizeof(this->next_node_addr));
    mem += sizeof(this->next_node_addr);

    memcpy(mem, &this->side_node_addr, sizeof(this->side_node_addr));
    mem += sizeof(this->side_node_addr);

    memcpy(mem, &this->eip, sizeof(this->eip));
    mem += sizeof(this->eip);

    // content
    size_t cmem_size = this->content.size() + 1;
    memcpy(mem, this->content.c_str(), cmem_size);
    mem += cmem_size;

    // api_reporter
    cmem_size = this->api_reporter.size() + 1;
    memcpy(mem, this->api_reporter.c_str(), cmem_size);
    mem += cmem_size;

    memcpy(mem, &this->branch_type, sizeof(this->branch_type));
    mem += sizeof(this->branch_type);
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
