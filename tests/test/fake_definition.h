#pragma once

#include <functional>
#include <goliath/definition/generate.h>
#include <string_view>

struct fake_definition : public definition::definition {
    fake_definition() = default;
    ~fake_definition() = default;

    void execute() const override;
    std::string_view string() const override;
    std::function<void()> _execute = nullptr;
};