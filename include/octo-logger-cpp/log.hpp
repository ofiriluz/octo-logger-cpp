/**
 * @file log.h
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef LOG_HPP_
#define LOG_HPP_

#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/logger-test-definitions.hpp"
#include <fmt/format.h>
#include <fmt/printf.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace octo::logger
{
class Logger;
class Log
{
  public:
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

  private:
    std::optional<std::ostringstream> stream_;
    LogLevel log_level_;
    const Logger& logger_;
    std::chrono::time_point<std::chrono::system_clock> time_created_;
    std::string extra_identifier_;
    ContextInfo context_info_;

  private:
    Log(const LogLevel& log_level, std::string_view extra_identifier, ContextInfo&& context_info, const Logger& logger);

  public:
    virtual ~Log();

    static std::string level_to_string(const LogLevel& level);
    static LogLevel string_to_level(const std::string& level_str);

    const std::chrono::time_point<std::chrono::system_clock>& time_created() const;
    [[nodiscard]] bool has_stream() const;
    // @brief Get the string representation of the log message. Caller must first check if the log has a stream.
    [[nodiscard]] std::string str() const;
    const LogLevel& log_level() const;
    const std::string& extra_identifier() const;
    ContextInfo const& context_info() const
    {
        return context_info_;
    }

    template <class T>
    Log& operator<<(const T& value)
    {
        if (stream_)
        {
            *stream_ << value;
        }
        return *this;
    }

    template <typename... T>
    void formatted(fmt::format_string<T...> fmt, T&&... args)
    {
        if (stream_)
        {
            *stream_ << fmt::format(fmt, std::forward<T>(args)...);
        }
    }

    template <typename... Args>
    void formattedf(char const* fmt, Args... args)
    {
        if (stream_)
        {
            *stream_ << fmt::sprintf(fmt, args...);
        }
    }

    friend class Logger;

    TESTS_MOCK_CLASS(Log)
};
} // namespace octo::logger

#endif
