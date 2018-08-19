#pragma once

#include <stdexcept>
#include <string>

namespace error
{
class win32 : public std::runtime_error
{
private:
    std::string context() const noexcept;

public:
    win32(std::string message) : std::runtime_error(context() + message)
    {
    }
    ~win32() = default;
    const char *what() const override;
};
}; // namespace error
