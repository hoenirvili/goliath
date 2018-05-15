#pragma once

#include "types.hpp"

/**
* Instruction type holds all context
* of the next instruction
*/
class Instruction {

public:
	size_t eip;					/* instruction pointer */
	const char* content;		/* complete instruction */
	Int32 branch_type;			/* no branch = 0 */
	size_t len;					/* instruction length */
	size_t argument_value;		/* instruction branch argument */

	/**
	* validate returns true if the
	* instruction object contains valid value
	* fields
	*/
	bool validate() const;

	/**
	* is_branch returns true if the
	* instruction is a instruction that can branch
	*/
	bool is_branch() const noexcept;

	/**
	* true_branch returns the address that
	* the branch will jump if it evaluates to true
	*/
	size_t true_branch() const noexcept;

	/**
	* false_branch returns the address that
	* the branch will jump if it evaluates to false
	*/
	size_t false_branch() const noexcept;
};

