#include "cfgtrace/node.h"
#include "cfgtrace/error/error.h"
#include "cfgtrace/format/string.h"
#include "cfgtrace/instruction.h"
#include <string>

using namespace std;

void Node::mark_done() noexcept
{
    this->is_done = true;
}

bool Node::done() const noexcept
{
    return this->is_done;
}

size_t Node::true_neighbour() const noexcept
{
    return this->true_branch_address;
}

size_t Node::false_neighbour() const noexcept
{
    return this->false_branch_address;
}

void Node::append_instruction(instruction instruction) noexcept
{
    size_t eip = instruction.pointer_address();
    // TODO(hoenir): always increment when first eip, ignore the rest
    // TODO(hoenir): what if the jump is in the middle of the block?
    if (this->done() && this->contains_address(eip)) {
        this->already_visited(eip);
        return;
    }

    this->block.push_back(instruction);
}

void Node::append_branch_instruction(instruction instruction) noexcept
{
    this->append_instruction(instruction);

    if (!instruction.is_ret()) {
        this->true_branch_address = instruction.true_branch_address();
        this->false_branch_address = instruction.false_branch_address();
    }

    this->mark_done();
}

void Node::already_visited(size_t eip) noexcept
{
    auto instruction = this->block[0];
    if (instruction.pointer_address() == eip)
        this->occurrences++;
}

bool Node::contains_address(size_t eip) const noexcept
{
    for (const auto &instruction : this->block)
        if (instruction.pointer_address() == eip)
            return true;

    return false;
}

bool Node::no_branching() const noexcept
{
    // TODO(): Modify this. terminal_node
    return (!this->true_branch_address && !this->false_branch_address && this->block.size());
}

/**
 * ARRAY_SIZE returns the number of elements that an array has
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// pallet contains all the blues9 color scheme pallet
static const unsigned int pallet[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

static double inline precent(double is, double of) noexcept
{
    return ((is / of) * 100);
}
/**
 * pick_color
 * for a given number of max occurrences and a fixed value
 * return the color pallet number in graphviz format
 */
static unsigned int pick_color(unsigned int max, unsigned int value) noexcept
{
    if ((max == 1) && (value == 1))
        return 1;

    size_t n = ARRAY_SIZE(pallet);
    const double split_in = 100.0 / n;
    auto interval = vector<double>(n);

    for (size_t i = 0; i < n; i++) {
        interval[i] = split_in * (i + 1);
        if (interval[i] == 100.0) // don't make this 100.0f
            interval[i]--;
    }

    const auto fvalue = static_cast<float>(value);
    const auto fmax = static_cast<float>(max);
    const auto p = precent(fvalue, fmax);

    if (p <= interval[0])
        return 1;
    if (p >= interval[n - 1])
        return 9;

    for (size_t i = 0; i < n - 1; i++) {
        auto low = interval[i];
        auto high = interval[i + 1];
        if (p >= low && p <= high) {
            const auto half = (high + low) / 2;
            const auto pr = precent(p, half);
            if (pr > 50.0)
                return pallet[i + 1];
            else
                return pallet[i];
        }
    }

    return 1;
}

size_t Node::start_address() const noexcept
{
    return this->_start_address;
}

string Node::graphviz_color() const noexcept
{
    if (this->no_branching())
        return "color = \"plum1\"";

    auto color = pick_color(this->max_occurrences, this->occurrences);
    auto str = "colorscheme = blues9\n\t\tcolor = " + to_string(color);
    if (color >= 7)
        str += "\n\t\tfontcolor = white";
    str += "\n";
    return str;
}

string Node::graphviz_name() const noexcept
{
    auto start = this->start_address();
    if (start)
        return format::string("0x%08X", start);
    return "";
}

string Node::graphviz_label() const noexcept
{
    string code_block = graphviz_name() + "\\l";

    if (this->block.size())
        code_block += "\\l";

    for (const auto &instruction : this->block) {
        string bl = instruction.str();
        code_block += bl + "\\l";
    }

    return "label = \"" + code_block + "\"";
}

static const char *graphviz_definition_template = R"(
	"%s" [
		%s
		%s
	]
)";

string Node::graphviz_definition() const
{
    const auto name = this->graphviz_name();
    const auto label = this->graphviz_label();
    const auto color = this->graphviz_color();

    const auto nm = name.c_str();
    const auto blk = label.c_str();
    const auto colr = color.c_str();

    return format::string(graphviz_definition_template, nm, blk, colr);
}

static inline string relation(size_t start, size_t end)
{
    return format::string("\"0x%08X\" -> \"0x%08X\"", start, end);
}

string Node::graphviz_relation() const
{
    string str = "";

    size_t start = this->start_address();
    if (this->true_branch_address) {
        str += relation(start, this->true_branch_address);
        str += " [color=green penwidth=2.0] \n";
    }

    if (this->false_branch_address) {
        str += relation(start, this->false_branch_address);
        str += " [color=red penwidth=2.0] \n";
    }

    return str;
}

bool Node::it_fits(size_t size) const noexcept
{
    return (size >= this->mem_size());
}

void Node::load_from_memory(const std::byte *mem) noexcept
{
    memcpy(&this->_start_address, mem, sizeof(this->_start_address));
    mem += sizeof(this->_start_address);

    // how many instruction we have?
    size_t n = 0;
    memcpy(&n, mem, sizeof(n));
    mem += sizeof(n);

    for (auto i = 0u; i < n; i++) {
        auto instr = instruction();
        instr.load_from_memory(mem);
        mem += instr.mem_size();
        // push the newly completed instruction back
        this->block.push_back(instr);
    }

    memcpy(&this->is_done, mem, sizeof(this->is_done));
    mem += sizeof(this->is_done);

    memcpy(&this->max_occurrences, mem, sizeof(this->max_occurrences));
    mem += sizeof(this->max_occurrences);

    memcpy(&this->true_branch_address, mem, sizeof(this->true_branch_address));
    mem += sizeof(this->true_branch_address);

    memcpy(&this->false_branch_address, mem, sizeof(this->false_branch_address));
    mem += sizeof(this->false_branch_address);

    memcpy(&this->occurrences, mem, sizeof(this->occurrences));
    mem += sizeof(this->occurrences);
}

void Node::load_to_memory(std::byte *mem) const noexcept
{
    memcpy(mem, &this->_start_address, sizeof(this->_start_address));
    mem += sizeof(this->_start_address);

    const size_t n = this->block.size();
    memcpy(mem, &n, sizeof(n));
    mem += sizeof(n);

    for (const auto &item : this->block) {
        item.load_to_memory(mem);
        mem += item.mem_size();
    }

    memcpy(mem, &this->is_done, sizeof(this->is_done));
    mem += sizeof(this->is_done);

    memcpy(mem, &this->max_occurrences, sizeof(this->max_occurrences));
    mem += sizeof(this->max_occurrences);

    memcpy(mem, &this->true_branch_address, sizeof(this->true_branch_address));
    mem += sizeof(this->true_branch_address);

    memcpy(mem, &this->false_branch_address, sizeof(this->false_branch_address));
    mem += sizeof(this->false_branch_address);

    memcpy(mem, &this->occurrences, sizeof(this->occurrences));
    mem += sizeof(this->occurrences);
}

size_t Node::mem_size() const noexcept
{
    size_t size = sizeof(this->_start_address);
    size += sizeof(this->block.size());
    for (const auto &item : this->block) {
        auto item_size = item.mem_size();
        size += sizeof(item_size);
        size += item_size;
    }
    size += sizeof(this->is_done);
    size += sizeof(this->max_occurrences);
    size += sizeof(this->true_branch_address);
    size += sizeof(this->false_branch_address);
    size += sizeof(this->occurrences);
    return size;
}
