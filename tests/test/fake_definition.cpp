#include "fake_definition.h"

#include <string_view>

void fake_definition::execute() const
{
    if (!this->_execute)
        return;

    this->_execute();
}

std::string_view fake_definition::string() const
{
    return "";
}