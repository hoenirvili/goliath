#include "custom_params.h"
#include "cfgtrace/api/types.h"
#include <algorithm>

custom_params::custom_params(size_t EIP,
                             const char *CompleteInstr,
                             BRANCH_TYPE BranchType,
                             size_t instrlen,
                             size_t next_addr,
                             size_t side_addr)
{
    this->params = new CUSTOM_PARAMS();
    params->MyDisasm = new DISASM();
    params->MyDisasm->EIP = EIP;
    auto begin = CompleteInstr;
    auto end = CompleteInstr + strlen(CompleteInstr) + 1;
    auto to = params->MyDisasm->CompleteInstr;
    std::copy(begin, end, params->MyDisasm->CompleteInstr);
    params->MyDisasm->Instruction.BranchType = BranchType;
    params->instrlen = instrlen;
    params->next_addr = next_addr;
    params->side_addr = side_addr;
}

custom_params::custom_params(const custom_params &cp)
{
    this->params->MyDisasm->EIP = cp.params->MyDisasm->EIP;
    auto start = cp.params->MyDisasm->CompleteInstr;
    auto end = start + sizeof(start);
    std::copy(start, end, this->params->MyDisasm->CompleteInstr);
    this->params->MyDisasm->Instruction.BranchType = cp.params->MyDisasm->Instruction.BranchType;
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
