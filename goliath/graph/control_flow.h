#pragma once

#include "goliath/assembly/instruction.h"
#include "goliath/graph/graph.h"
#include "goliath/graph/node.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace graph
{
	/**
	 * control_flow graph is a type of graph that
	 * holds instruction and can represent the flow of an execution
	 */
	class control_flow : public graph
	{
	public:
		using node_ptr = std::unique_ptr<Node>;
		/**
		 * start_address_first_node is the start address of the first
		 * instruction that has been added into our control flow graph
		 */
		size_t start_address_first_node = 0;

		/**
		 * nodes a map of start node addresses and pointers to nodes
		 * in each and every node we can hold multiple instruction based on the
		 * context we are in
		 */
		mutable std::map<size_t, node_ptr> nodes;

		/**
		 *  read from a block of memory that we assume holds information from
		 *  a control flow graph and try to initialise internal fields.
		 */
		void read(const std::byte *from) noexcept override;

		/**
		 *  write the current state of the control flow graph in memory pointed by
		 * to we assume that to is bigger enough to hold all the internal
		 * information
		 */
		void write(std::byte *to) const noexcept override;

		/**
		 *  append adds an assembly instruction in the control flow graph
		 */
		void append(assembly::instruction instruction, size_t iteration) override;

		/**
		 * generate generates out of the control flow graph an textual
		 * representation that a third program can compile and run
		 * to turn it into a png, jpeg or a viewable format by the user
		 * TODO(hoenir): MAKE THIS GENERATION SUPPORT GDL
		 */
		definition::definition *generate(definition::FORMAT format);

		control_flow() = default;
		~control_flow() = default;

	private:
		size_t current_node_start_addr = 0x0;
		size_t current_pointer = 0x0;

		size_t mem_size() const noexcept;
		bool node_exists(size_t start) const noexcept;

		std::string graphviz();
		std::string control_flow::gdl();
		bool node_contains_address(size_t address) const noexcept;
		void set_nodes_max_occurrences() noexcept;
		void unset_current_address(const node_ptr &node) noexcept;
		void append_node_neighbors(const node_ptr &node, size_t iteration) noexcept;
		bool it_fits(const size_t size) const noexcept;
		size_t set_and_get_current_address(size_t eip) noexcept;
		node_ptr get_current_node(size_t start_address, size_t iteration) noexcept;
		void append_instruction(assembly::instruction instruction,
			size_t current_iteration);
		void append_branch_instruction(assembly::instruction instruction,
			size_t iteration);
	};

}; // namespace graph
