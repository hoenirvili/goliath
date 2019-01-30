#include "goliath/command/gdl.h"
#include "goliath/error/error.h"
#include "goliath/random/random.h"

#include <fstream>
#include <string_view>

namespace command
{
	gdl::gdl(std::string_view definitions, size_t iteration)
	{
		this->definitions = definitions;
		this->iteration = iteration;
	}

	void gdl::execute() const
	{
		auto name = std::to_string(this->iteration) + "_" + random::string() + ".gdl";
		auto out = std::fstream(name, std::fstream::out);
		if (!out)
			throw ex(std::runtime_error, "fstream failed");

		out << this->definitions << std::endl;
	}

	std::string_view gdl::string() const
	{
		return this->definitions;
	}

}; // namespace command