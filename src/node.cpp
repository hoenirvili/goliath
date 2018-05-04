#include "node.hpp"
#include "common.hpp"

#include <string>

using namespace std;

// pallete contains all the blues9 color scheme pallete
static const unsigned int pallete[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

/**
 * pick_color
 * for a given number of occurences return a number
 * based on the color pallete numbers in graphviz
 */
static string pick_color(unsigned int occurences)
{
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
    /*
        definitions for the node
        1-%s name node
        2-%s asm blocks
        3-%s heatmap color
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
