#pragma once

#include <cfgtrace/definition/generate.h>

#include <functional>

struct fake_definition : public definition::definition
{
    fake_definition() = default;
    ~fake_definition() = default;

    void execute() const override;

    std::function<void()>_execute = nullptr;
};