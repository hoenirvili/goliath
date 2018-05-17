#pragma once

#include "types.hpp"

/**
* Instruction type holds all context of the next instruction
*/
class Instruction {

private:
	const size_t len;			/* instruction length */
	const size_t argument_value;/* instruction branch argument */

public:
	const size_t eip;			/* instruction pointer */
	const char* content;		/* complete instruction */
	const Int32 branch_type;	/* no branch = 0 */
	
	Instruction(
		size_t eip,
		char* content,
		Int32 branch_type,
		size_t len,
		size_t argument_value 
	) noexcept :
		eip(eip),
		content(content),
		branch_type(branch_type),
		len(len),
		argument_value(argument_value) {}

	~Instruction() = default;

	/**
	* ret_type returns true if  is a ret instruction
	*/
	bool is_ret() const noexcept;

	/**
	* validate returns true if the
	* instruction object contains valid values
	*/
	bool validate() const noexcept;

	/**
	* is_branch returns true if the
	* instruction is an instruction that can branch
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

