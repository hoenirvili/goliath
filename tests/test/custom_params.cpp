#include "custom_params.h"

#include <goliath/api/types.h>

#include <algorithm>

custom_params::custom_params(size_t EIP,
                             const char *CompleteInstr,
                             BRANCH_TYPE BranchType,
                             size_t instrlen,
                             size_t next_addr,
                             size_t side_addr)
{
    this->params = new CUSTOM_PARAMS();
    this->params->MyDisasm = new DISASM();
    this->params->MyDisasm->EIP = EIP;

    auto begin = CompleteInstr;
    auto end = CompleteInstr + strlen(CompleteInstr);
    auto to = this->params->MyDisasm->CompleteInstr;
    std::copy(begin, end, to);

    this->params->MyDisasm->Instruction.BranchType = BranchType;
    this->params->instrlen = instrlen;
    this->params->next_addr = next_addr;
    this->params->side_addr = side_addr;
}

custom_params::custom_params(const custom_params &cp)
{
    this->params = new CUSTOM_PARAMS();
    this->params->MyDisasm = new DISASM();
    this->params->MyDisasm->EIP = cp.params->MyDisasm->EIP;
    auto start = cp.params->MyDisasm->CompleteInstr;
    auto end = start + sizeof(cp.params->MyDisasm->CompleteInstr);
    std::copy(start, end, this->params->MyDisasm->CompleteInstr);
    this->params->MyDisasm->Instruction.BranchType =
      cp.params->MyDisasm->Instruction.BranchType;
    this->params->instrlen = cp.params->instrlen;
    this->params->next_addr = cp.params->next_addr;
    this->params->side_addr = cp.params->side_addr;
}

CUSTOM_PARAMS *custom_params::get() const noexcept
{
    return this->params;
}

custom_params::~custom_params()
{
    if (!this->params)
        return;

    delete this->params->MyDisasm;
    delete this->params;

    this->params = nullptr;
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

static inline bool _is_call(BRANCH_TYPE type) noexcept
{
    return type == CallType;
}

static inline bool _is_ret(BRANCH_TYPE type) noexcept
{
    return type == RetType;
}

bool custom_params::branch() const
{
    auto a = _is_branch(
      static_cast<BRANCH_TYPE>(this->params->MyDisasm->Instruction.BranchType));

    auto b = _is_call(
      static_cast<BRANCH_TYPE>(this->params->MyDisasm->Instruction.BranchType));

    auto c = _is_ret(
      static_cast<BRANCH_TYPE>(this->params->MyDisasm->Instruction.BranchType));

    return a || b || c;
}
