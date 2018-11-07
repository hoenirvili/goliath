#include "fake_definition.h"

void fake_definition::execute() const
{
    if (!this->_execute)
        return;

    this->_execute();
}