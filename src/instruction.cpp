#include "instruction.hpp"
#include "log.hpp"

bool Instruction::is_branch() const noexcept
{
	switch (this->branch_type) {
	case JO:
	case JC:
	case JE:
	case JA:
	case JS:
	case JP:
	case JL:
	case JG:
	case JB:
	case JECXZ:
	case JmpType:
	case CallType:
	case RetType:
	case JNO:
	case JNC:
	case JNE:
	case JNA:
	case JNS:
	case JNP:
	case JNL:
	case JNG:
	case JNB:
		return true;
	}

	return false;
}

size_t Instruction::true_branch() const noexcept
{
	return this->argument_value;
}

size_t Instruction::false_branch() const noexcept
{
	return this->eip + this->len;
}

bool Instruction::is_ret() const noexcept
{
	return (this->branch_type == RetType);
}

bool Instruction::validate() const noexcept
{
	if (this->content == nullptr) {
		log_warning("empty instruction content");
		return false;
	}

	if (!this->is_branch() && 
		this->branch_type != 0) {
		log_warning("invalid branch passed %d", this->branch_type);
		return false;
	}

	if (this->eip == 0) {
		log_warning("invalid eip instruction %d", this->eip);
		return false;
	}

	if (this->len == 0) {
		log_warning("invalid instruction len : %d", this->len);
		return false;
	}

	return true;
}