/**
 * @file log.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include <algorithm>

namespace octo::logger
{

Log::Log(const Log::LogLevel& log_level,
         std::string_view extra_identifier,
         ContextInfo&& context_info,
         const Logger& logger)
    : stream_(nullptr),
      log_level_(log_level),
      logger_(logger),
      extra_identifier_(extra_identifier),
      context_info_(std::move(context_info))
{
    if (log_level_ >= logger.logger_channel().log_level() && log_level_ != LogLevel::QUIET)
    {
        stream_ = std::make_unique<std::ostringstream>();
    }
}

Log::~Log()
{
    if (stream_)
    {
        time_created_ = std::chrono::system_clock::now();
        logger_.dump_log(*this);
        stream_.reset();
    }
}

std::string Log::level_to_string(const Log::LogLevel& level)
{
    switch (level)
    {
        case Log::LogLevel::QUIET:
            return "Quiet";
        case Log::LogLevel::TRACE:
            return "Trace";
        case Log::LogLevel::DEBUG:
            return "Debug";
        case Log::LogLevel::INFO:
            return "Info";
        case Log::LogLevel::NOTICE:
            return "Notice";
        case Log::LogLevel::WARNING:
            return "Warning";
        case Log::LogLevel::ERROR:
            return "Error";
    }

    throw std::runtime_error("Invalid Log Level");
}

Log::LogLevel Log::string_to_level(const std::string& level_str)
{
    auto level_up_str = level_str;
    std::transform(level_up_str.begin(), level_up_str.end(), level_up_str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    if (level_up_str == "QUIET")
    {
        return Log::LogLevel::QUIET;
    }
    else if (level_up_str == "TRACE")
    {
        return Log::LogLevel::TRACE;
    }
    else if (level_up_str == "DEBUG")
    {
        return Log::LogLevel::DEBUG;
    }
    else if (level_up_str == "INFO")
    {
        return Log::LogLevel::INFO;
    }
    else if (level_up_str == "NOTICE")
    {
        return Log::LogLevel::NOTICE;
    }
    else if (level_up_str == "WARNING")
    {
        return Log::LogLevel::WARNING;
    }
    else if (level_up_str == "ERROR")
    {
        return Log::LogLevel::ERROR;
    }

    throw std::runtime_error("Invalid Log Level String");
}

const std::chrono::time_point<std::chrono::system_clock>& Log::time_created() const
{
    return time_created_;
}

const std::ostringstream* Log::stream() const
{
    return stream_.get();
}

const Log::LogLevel& Log::log_level() const
{
    return log_level_;
}

const std::string& Log::extra_identifier() const
{
    return extra_identifier_;
}
} // namespace octo::logger
