#pragma once

#include "goliath/definition/generate.h"

#include <string>

namespace command
{
	/**
	 * gdl type used to store all gdl context information
	 * for generating a graph
	 */
	struct gdl : public definition::definition {
		/**
		 * create a new graphviz graph at a certain iteration based
		 * on the definitions provided
		 */
		gdl(std::string_view definitions, size_t iteration);
		~gdl() = default;

		/**
		 * execute compiles the gdl graph into a gdl file
		 */
		void execute() const override;

		/**
		 * return the gdl format definitions as a string
		 */
		std::string_view string() const override;

	private:
		std::string definitions;
		size_t iteration;
	};

}; // namespace command