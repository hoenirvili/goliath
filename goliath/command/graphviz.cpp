#include "goliath/command/graphviz.h"
#include "goliath/command/execute.h"
#include "goliath/error/error.h"
#include "goliath/random/random.h"

#include <fstream>
#include <stdexcept>
#include <string_view>

namespace command
{
graphviz::graphviz(std::string_view definitions, size_t iteration)
{
    this->definitions = definitions;
    this->iteration = iteration;
}

void graphviz::execute() const
{
    auto out = std::fstream("partiaflowgraph.dot", std::fstream::out);
    if (!out)
        throw ex(std::runtime_error, "fstream failed");

    out << this->definitions << std::endl;

    auto name = std::to_string(this->iteration) + "_" + random::string();

    const std::string cmd = "dot -Tpng partiaflowgraph.dot -o" + name + ".png";
    std::string process_stderr, process_exit;

    command::execute(cmd, &process_stderr, &process_exit);

    std::string exception_message = "";
    if (!process_stderr.empty())
        exception_message += process_exit;

    if (!process_exit.empty())
        exception_message += " " + process_exit;

    if (!exception_message.empty())
        throw ex(std::runtime_error, exception_message);
}

std::string_view graphviz::string() const
{
    return this->definitions;
}

}; // namespace command