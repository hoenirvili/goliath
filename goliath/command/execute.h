#pragma once

#include <string>
#include <string_view>

namespace command
{
/**
 * execute executes the command specified in the command parameter
 * If the command is encounters an error, all the content that is written
 * in stderr it will be piped into process_stderr. If the command returns
 * and exit code, the exit code will be translated into an human readable
 * message and it will be written into the process_exit, specified by the user
 */
void execute(std::string_view command,
             std::string *process_stderr,
             std::string *process_exit);
}; // namespace command