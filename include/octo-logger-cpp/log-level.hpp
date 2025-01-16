/**
 * @file log-level.hpp
 * @author Denis Sheyer (denis.sheyer@gmail.com)
 * @brief
 * @date 12025-01-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef LOG_LEVEL_HPP_
#define LOG_LEVEL_HPP_

#include <algorithm>
#include <cinttypes>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace octo::logger
{
enum class LogLevel : std::uint8_t
{
    TRACE = 1,
    DEBUG = 2,
    INFO = 4,
    NOTICE = 8,
    WARNING = 16,
    ERROR = 32,
    QUIET = 255,
};

class LogLevelUtils
{
  public:
    LogLevelUtils() = delete;
    ~LogLevelUtils() = delete;

    /**
     * @brief Convert a LogLevel to a Capitalized string.
     * @param level The LogLevel to convert.
     */
    static inline std::string const& level_to_string(LogLevel level) noexcept
    {
        static std::unordered_map<LogLevel, std::string> const level_names{
            {LogLevel::QUIET, "Quiet"},
            {LogLevel::TRACE, "Trace"},
            {LogLevel::DEBUG, "Debug"},
            {LogLevel::INFO, "Info"},
            {LogLevel::NOTICE, "Notice"},
            {LogLevel::WARNING, "Warning"},
            {LogLevel::ERROR, "Error"},
        };
        return level_names.at(level);
    }

    /**
     * @brief Convert a LogLevel to a lowercase string.
     * @param level The LogLevel to convert.
     */
    static inline std::string const& level_to_string_lower(LogLevel level) noexcept
    {
        static std::unordered_map<LogLevel, std::string> const level_names{
            {LogLevel::QUIET, "quiet"},
            {LogLevel::TRACE, "trace"},
            {LogLevel::DEBUG, "debug"},
            {LogLevel::INFO, "info"},
            {LogLevel::NOTICE, "notice"},
            {LogLevel::WARNING, "warning"},
            {LogLevel::ERROR, "error"},
        };
        return level_names.at(level);
    }

    /**
     * @brief Convert a LogLevel to an UPPERCASE string.
     * @param level The LogLevel to convert.
     */
    static inline std::string const& level_to_string_upper(LogLevel level) noexcept
    {
        static std::unordered_map<LogLevel, std::string> const level_names{
            {LogLevel::QUIET, "QUIET"},
            {LogLevel::TRACE, "TRACE"},
            {LogLevel::DEBUG, "DEBUG"},
            {LogLevel::INFO, "INFO"},
            {LogLevel::NOTICE, "NOTICE"},
            {LogLevel::WARNING, "WARNING"},
            {LogLevel::ERROR, "ERROR"},
        };
        return level_names.at(level);
    }

    /**
     * @brief Convert a LogLevel to an UPPERCASE short string.
     * @param level The LogLevel to convert.
     */
    static inline std::string const& level_to_string_short(LogLevel level) noexcept
    {
        static std::unordered_map<LogLevel, std::string> const level_names{
            {LogLevel::QUIET, "Q"},
            {LogLevel::TRACE, "T"},
            {LogLevel::DEBUG, "D"},
            {LogLevel::INFO, "I"},
            {LogLevel::NOTICE, "N"},
            {LogLevel::WARNING, "W"},
            {LogLevel::ERROR, "E"},
        };
        return level_names.at(level);
    }

    /**
     * @brief Convert a string to a LogLevel.
     *
     * The input string should be lowercase/UPPERCASE/Capitalized for efficiency.
     * If the input string is in MiXeDcAsE, it will be normalized to UPPERCASE first which is less efficient.
     *
     * @param level_str The string to convert.
     */
    static inline LogLevel string_to_level(std::string const& level_str) noexcept(false)
    {
        static std::unordered_map<std::string, LogLevel> const level_names{
            {"QUIET", LogLevel::QUIET},     {"Quiet", LogLevel::QUIET},     {"quiet", LogLevel::QUIET},
            {"TRACE", LogLevel::TRACE},     {"Trace", LogLevel::TRACE},     {"trace", LogLevel::TRACE},
            {"DEBUG", LogLevel::DEBUG},     {"Debug", LogLevel::DEBUG},     {"debug", LogLevel::DEBUG},
            {"INFO", LogLevel::INFO},       {"Info", LogLevel::INFO},       {"info", LogLevel::INFO},
            {"NOTICE", LogLevel::NOTICE},   {"Notice", LogLevel::NOTICE},   {"notice", LogLevel::NOTICE},
            {"WARNING", LogLevel::WARNING}, {"Warning", LogLevel::WARNING}, {"warning", LogLevel::WARNING},
            {"ERROR", LogLevel::ERROR},     {"Error", LogLevel::ERROR},     {"error", LogLevel::ERROR},
        };
        if (auto const itr = level_names.find(level_str); itr != level_names.cend())
        {
            return itr->second;
        }

        // We may have gotten a weird mixed case, normalize it and try again.
        std::string level_up_str(level_str);
        std::transform(level_up_str.begin(), level_up_str.end(), level_up_str.begin(), [](unsigned char const c) {
            return std::toupper(c);
        });
        if (auto const itr = level_names.find(level_up_str); itr != level_names.cend())
        {
            return itr->second;
        }

        throw std::runtime_error("Invalid Log Level String");
    }
};

} // namespace octo::logger

#endif // LOG_LEVEL_HPP_
