#pragma once

#include <string>
#include <memory>
#include <unordered_map>

class Log {

private:

    enum class level{error, info, warning};

    std::unordered_map<level, const std::string> m_prefix = 
    {
        {level::error, "ERROR"},
        {level::warning, "WARNING"},
        {level::info, "INFO"},
    };

    const std::string& context;

public:

    Log(const std::string& ctx, std::ostream* os = nullptr);
    ~Log() = default;

    void error(const std::string& message) const noexcept;
    void warning(const std::string& message) const noexcept;
    void info(const std::string& message) const noexcept;
};
