#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <ostream>

/**
 * Log class that can log messages to files, streams
 * using different levels of output
 */
class Log {

private:

	using unique_writer = std::unique_ptr<std::ostream>;

    /**
     * level represents different levels of logging
     */
    enum class level:std::uint8_t{error, info, warning};

    /**
     * m_prefix list of level and their string representation
     */
    std::unordered_map<level, const std::string> m_prefix = 
    {
        {level::error, "ERROR"},
        {level::warning, "WARNING"},
        {level::info, "INFO"},
    };

    /**
     * w is the internal writer that the Log will use
     * in order to output logging messages
     */
	unique_writer w;

public:
	Log() = default;
	~Log() = default;

    /**
     *  instance given a output stream return a new log instance
     */
	static std::shared_ptr<Log> instance(std::ostream* os = nullptr) noexcept;
    /**
     *  instance given an log name return a new log instance using
     *  the default file file implementation
     */
	static std::shared_ptr<Log> instance(const std::string& name) noexcept;

    /**
     *  redirect can redirect the logging output to any
     *  other output stream
     */
	void redirect(std::ostream* os) noexcept;

    /**
     * error writes error logging messages
     */
    void error(const std::string& message) const noexcept;

     /**
     * warning writes warning logging messages
     */
    void warning(const std::string& message) const noexcept;

    /**
     * info writes info logging messages
     */
    void info(const std::string& message) const noexcept;
};