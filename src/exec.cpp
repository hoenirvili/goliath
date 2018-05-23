#include "exec.hpp"

using namespace std;

string execute_command(const string &command)
{
	char buffer[1024] = { 0 };
	string str = "";
	auto pipe = _popen(command.c_str(), "rt");
	if (!pipe)
		return "_popen returns an invalid file pointer";

	
	while (fgets(buffer, 1024, pipe) != NULL)
		str += buffer;

	int err = feof(pipe);
	if (!err)
		return "failed to read the pipe to the end";

	_pclose(pipe);

	return str;
}
