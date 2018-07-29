#pragma once

#include <string>

void execute_command(
	const std::string& command, 
	std::string* process_stderr, 
	std::string* process_exit
);
