#pragma once

#include <string>

namespace command
{
void execute(const std::string &command,
             std::string *process_stderr,
             std::string *process_exit);
};