#pragma once

#include <string>
#include <string_view>

namespace command
{
void execute(std::string_view command, std::string *process_stderr, std::string *process_exit);
};