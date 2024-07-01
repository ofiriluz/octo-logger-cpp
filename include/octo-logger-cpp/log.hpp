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
#include <memory>
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
    std::unique_ptr<std::ostringstream> stream_;
    LogLevel log_level_;
    const Logger& logger_;
    std::chrono::time_point<std::chrono::system_clock> time_created_;
    std::string extra_identifier_;
    ContextInfo context_info_;

  private:
    Log(const LogLevel& log_level, std::string_view extra_identifier, const Logger& logger);

  public:
    virtual ~Log();

    static std::string level_to_string(const LogLevel& level);
    static LogLevel string_to_level(const std::string& level_str);

    const std::chrono::time_point<std::chrono::system_clock>& time_created() const;
    const std::ostringstream* stream() const;
    const LogLevel& log_level() const;
    const std::string& extra_identifier() const;
    const ContextInfo& context_info() const
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
    Log& formatted(fmt::format_string<T...> fmt, T&&... args)
    {
        if (stream_)
        {
            *stream_ << fmt::format(fmt, std::forward<T>(args)...);
        }
        return *this;
    }

    Log& with_context(ContextInfo context_info)
    {
        context_info_.update(std::move(context_info));
        return *this;
    }

    // Log& with_context(ContextInfo::ContextInfoInitializerList context_info)
    // {
    //     context_info_.update(ContextInfo(context_info));
    //     return *this;
    // }

    friend class Logger;

    TESTS_MOCK_CLASS(Log)
};
} // namespace octo::logger

#endif
