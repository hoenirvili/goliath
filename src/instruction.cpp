#include "instruction.h"
#include "log.h"
#include "api.h"

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

const char* Instruction::string() const noexcept
{
	return this->content;
}

size_t Instruction::true_branch_address() const noexcept
{
	if (this->is_branch())
		return this->next_node_addr;
	return this->eip + this->len;
}

size_t Instruction::false_branch_address() const noexcept
{
	if (this->is_branch() && !this->is_ret() && !this->direct_branch()) {
		if (this->next_node_addr == this->side_node_addr)
			return this->eip + this->len;
		return this->side_node_addr;
	}
	return 0;
}

bool Instruction::direct_branch() const noexcept
{
	return (this->branch_type == JmpType);
}

size_t Instruction::pointer_address() const noexcept
{
	return this->eip;
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

	if (this->next_node_addr == 0) {
		log_warning("invalid next node address : %d", this->next_node_addr);
		return false;
	}

	return true;
}