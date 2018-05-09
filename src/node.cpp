#include "node.hpp"
#include "common.hpp"

#include <cstring>
#include <string>

using namespace std;

// pallete contains all the blues9 color scheme pallete
static const unsigned int pallete[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

/**
 * pick_color
 * for a given number of occurences return a number
 * based on the color pallete numbers in graphviz
 */
static string pick_color(unsigned int occurences) {
    const unsigned int interval[] = {1, 7, 14, 28, 40, 70, 80, 100, 120};

    string color = "1";
    size_t n = sizeof(interval) / sizeof(interval[0]);
    unsigned int di = 0, dj = 0;

	unsigned int i = 0;
	unsigned int j = i + 1;
	for (; i < n - 1 && j < n; i++, j++) {
		if ((interval[i] >= occurences) && (interval[j] <= occurences)) {
			di = interval[i] - occurences;
			dj = occurences - interval[j];
			bool flag = (di > dj);
			switch (flag) {
			case true:
				color = std::to_string(pallete[i]);
				break;
			case false:
				color = std::to_string(pallete[j]);
				break;
			}
		}
	}

    return color;
}

string Node::graphviz_definition() const
{
    constexpr int size = 4096;
    /**
     *   definitions for the node
     *   1-%s name node
     *   2-%s asm blocks
     *   3-%s heatmap color
    */
    constexpr const char* format = R"(
	%s [
		label = "%s"
		color = "%s"
	]
)";

    auto name = "node" + std::to_string(this->start_address);
    
	string block = "";
    for (string bl : this->block)
        block += bl + "\\n";

    auto color = pick_color(this->occurences);
    
	const auto nm = name.c_str();
    const auto blk = block.c_str();
    const auto colr = color.c_str();
	
	return string_format(format, nm, blk, colr);
}

static string relation(const size_t start, const size_t end)
{
	return string_format("node%lu -> node%lu", start, end);
}

string Node::graphviz_relation() const
{
	return
		relation(this->start_address, this->true_branch_address) +
		"\\n" +
		relation(this->start_address, this->false_branch_address) +
		"\\n";
}

bool Node::it_fits(size_t size) const noexcept
{
	return (size >= this->mem_size());
}

int Node::deserialize(const uint8_t *mem, const size_t size) noexcept
{
	if ((!mem) || (!size))
		return EINVAL;
	
	if (!this->it_fits(size))
		return ENOMEM;

	memcpy(&this->start_address, mem, sizeof(this->start_address));
	mem += sizeof(this->start_address);

	memcpy(&this->occurences, mem, sizeof(this->occurences));
	mem += sizeof(this->occurences);

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
	if ((!mem) || (!size))
		return EINVAL;

	if (!this->it_fits(size))
		return ENOMEM;
	
	/**
	*	- first 4 bytes are the start_addr of the node
	*	- second 4 bytes are the number of occurences
	*	- third 4 bytes true branch address
	*	- forth 4 bytes false branch address
	*	- fifth 4 bytes is the size of the node block
	*	- the last is the hole block as strings, every string  is separated by "\0"
	*	so we don't need to include his size also
	*/

	memcpy(mem, &this->start_address, sizeof(this->start_address));
	mem += sizeof(this->start_address);
	
	memcpy(mem, &this->occurences, sizeof(this->occurences));
	mem += sizeof(this->occurences);
	
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
	size += sizeof(this->occurences);
	size += sizeof(this->true_branch_address);
	size += sizeof(this->false_branch_address);
	size += sizeof(this->block.size());
	for (const auto& item : this->block)
		size += item.size() + 1;

	return size;
}
