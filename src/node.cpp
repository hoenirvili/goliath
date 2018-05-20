#include <cstring>
#include <string>

#include "node.hpp"
#include "common.hpp"
#include "log.hpp"

using namespace std;

void Node::mark_done() noexcept
{
	this->is_done = true;
}

bool Node::done() const noexcept
{
	return this->is_done;
}

bool Node::no_branching() const noexcept
{
	return (!this->true_branch_address &&
		!this->false_branch_address &&
		!this->block.size());
}

std::string Node::graphviz_color() const noexcept
{
	if (this->no_branching())
		return "color = \"plum1\"";
	
	std::string color = "1";//TODO(hoenir): fix this
	return "colorscheme = blues9\n\t\tcolor=" + color + "\n";
}

std::string Node::graphviz_name() const noexcept
{
	if (this->start_address)
		return string_format("0x%08x", this->start_address);
	 
	return "";
}

std::string Node::graphviz_label() const noexcept
{
	string block = graphviz_name() + "\\n";

	if (this->block.size())
		block += "\\n";

	for (const auto &bl : this->block)
		block += bl + "\\n";

	block = "label = \"" + block + "\"";
	
	return block;
}


bool Node::validate() const noexcept
{
	if (!this->start_address) {
		log_error("node contains invalid start address : %lu", this->start_address);
		return false;
	}

	return true;
}

static const char* graphviz_definition_template = R"(
	"%s" [
		%s
		%s
	]
)";

string Node::graphviz_definition() const
{
	if (!this->validate())
		return "";
	
	const auto name = this->graphviz_name();
	const auto label = this->graphviz_label();
	const auto color = this->graphviz_color();
	
	const auto nm = name.c_str();
    const auto blk = label.c_str();
    const auto colr = color.c_str();
	
	return string_format(
		graphviz_definition_template, 
		nm, blk, colr);
}

static inline string relation(size_t start, size_t end)
{
	return string_format("\"0x%08x\" -> \"0x%08x\"", start, end);
}

string Node::graphviz_relation() const
{
	string str = "";
	if (this->true_branch_address) {
		str += relation(this->start_address, this->true_branch_address);
		str += " [color=green penwidth=3.5] \n";
	}

	if (this->false_branch_address) {
		str += relation(this->start_address, this->false_branch_address);
		str += " [color=red penwidth=3.5] \n";
	}

	return str;
}

bool Node::it_fits(size_t size) const noexcept
{
	return (size >= this->mem_size());
}

int Node::deserialize(const uint8_t *mem, const size_t size) noexcept
{
	if (!this->it_fits(size))
		return ENOMEM;

	memcpy(&this->start_address, mem, sizeof(this->start_address));
	mem += sizeof(this->start_address);

	memcpy(&this->occurrences, mem, sizeof(this->occurrences));
	mem += sizeof(this->occurrences);

	memcpy(&this->true_branch_address, mem, sizeof(this->true_branch_address));
	mem += sizeof(this->true_branch_address);

	memcpy(&this->false_branch_address, mem, sizeof(this->false_branch_address));
	mem += sizeof(this->false_branch_address);

	size_t block_size = 0;
	memcpy(&block_size, mem, sizeof(block_size));
	mem += sizeof(block_size);

	this->block.clear();

	char *content = NULL;
	size_t content_length = 0;
	
	for (size_t i = 0; i < block_size; i++) {
		content = (char*)mem;

		this->block.push_back(content);

		content_length = strlen((const char*)mem) + 1;
		mem += content_length;

	}

	return 0;
}

int Node::serialize(uint8_t *mem, const size_t size) const noexcept
{
	if (!this->it_fits(size))
		return ENOMEM;
	
	/**
	*	- first 4 bytes are the start_addr of the node
	*	- second 4 bytes are the number of occurrences
	*	- third 4 bytes true branch address
	*	- forth 4 bytes false branch address
	*	- fifth 4 bytes is the size of the node block
	*	- the last is the hole block as strings, every string  is separated by "\0"
	*	so we don't need to include his size also
	*/

	memcpy(mem, &this->start_address, sizeof(this->start_address));
	mem += sizeof(this->start_address);
	
	memcpy(mem, &this->occurrences, sizeof(this->occurrences));
	mem += sizeof(this->occurrences);
	
	memcpy(mem, &this->true_branch_address, sizeof(this->true_branch_address));
	mem += sizeof(this->true_branch_address);
	
	memcpy(mem, &this->false_branch_address, sizeof(this->false_branch_address));
	mem += sizeof(this->false_branch_address);
	
	auto block_size = this->block.size();
	memcpy(mem, &block_size, sizeof(block_size));
	mem += sizeof(block_size);
	
	for (const auto &item : this->block) {
		auto code = item.c_str();
		size_t code_size = strlen(code) + 1;
		
		memcpy(mem, code, code_size);
		mem += code_size;
	}

	return 0;
}

size_t Node::mem_size() const noexcept
{
	size_t size = 0;
	size += sizeof(this->start_address);
	size += sizeof(this->occurrences);
	size += sizeof(this->true_branch_address);
	size += sizeof(this->false_branch_address);
	size += sizeof(this->block.size());
	for (const auto& item : this->block)
		size += item.size() + 1;

	return size;
}
