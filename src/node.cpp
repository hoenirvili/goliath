#include "node.hpp"

#include <string>

// pallete contains all the
// blues9 color scheme pallete
static const unsigned int pallete[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

/**
 * pick_color
 * for the given number of occurences of a node this will select
 * based on the pallete a color number in graphviz format
 */
static std::string pick_color(unsigned int occurences)
{
    const unsigned int interval[] = {1, 7, 14, 28, 40, 70, 80, 100, 120};

    std::string color = "1";
    size_t n = sizeof(interval) / sizeof(interval[0]);
    unsigned int di = 0, dj = 0;

    for (unsigned int i = 0, j = i + 1; i < n - 1 && j < n; i++, j++)
        if ((interval[i] >= occurences) && (interval[j] <= occurences)) {
            di = interval[i] - occurences;
            dj = occurences - interval[j];
            bool flag = (di > dj);
            switch (flag) {
            case true:
                return color = std::to_string(pallete[i]);
                break;
            case false:
                color = std::to_string(pallete[j]);
                break;
            }
        }

    return color;
}

std::string Node::graphviz_definition()
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

    auto name = "node" + std::to_string(this->si);
    std::string block = "";

    // TODO(hoenir):this can be bigger than size
    for (std::string bl : this->block)
        block += bl + "\\n";

    auto color = pick_color(this->occurences);
    // TODO(hoenir):make sure 4096 it's big enough
    char buff[size] = {0};

    const char* nm = name.c_str();
    const char* blk = block.c_str();
    const char* colr = color.c_str();

    snprintf(buff, size, format, nm, blk, colr);
    return std::string(buff);
}

static inline std::string relation(size_t s, size_t e)
{
    auto start = std::to_string(s);
    auto end = std::to_string(e);
    return "node" + start + " -> " + "node" + end + "\n";
}

std::string Node::graphviz_realtion()
{
    return relation(this->si, this->tb) + relation(this->si, this->fb);
}
