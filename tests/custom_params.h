#pragma once

#include "cfgtrace/api/types.h"

const enum BRANCH_TYPE NO_BRANCH = static_cast<BRANCH_TYPE>(0);

class custom_params
{
public:
    custom_params(size_t EIP,
                  const char *CompleteInstr,
                  BRANCH_TYPE BranchType,
                  size_t instrlen,
                  size_t next_addr,
                  size_t side_addr);
    CUSTOM_PARAMS *get() const noexcept;
    custom_params(const custom_params &cp);
    ~custom_params();

private:
    CUSTOM_PARAMS *params = nullptr;
};