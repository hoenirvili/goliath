#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>

class win32_error : public std::runtime_error
{
private:
    std::string context() const noexcept;

public:
    win32_error(std::string message) : std::runtime_error(context() + message)
    {
    }
    ~win32_error() = default;
    const char *what() const override;
};
