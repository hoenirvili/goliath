#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <ostream>

class Log {

private:

	using unique_writer = std::unique_ptr<std::ostream>;

    enum class level:std::uint8_t{error, info, warning};

    std::unordered_map<level, const std::string> m_prefix = 
    {
        {level::error, "ERROR"},
        {level::warning, "WARNING"},
        {level::info, "INFO"},
    };

	unique_writer w;

public:

	Log() = default;
	~Log() = default;

	static std::shared_ptr<Log> instance(std::ostream*os = nullptr) noexcept;
	static std::shared_ptr<Log> instance(const std::string& name) noexcept;
	void redirect(std::ostream* os) noexcept;
    void error(const std::string& message) const noexcept;
    void warning(const std::string& message) const noexcept;
    void info(const std::string& message) const noexcept;
};