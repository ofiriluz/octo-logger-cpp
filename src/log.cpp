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
    : stream_(std::nullopt),
      log_level_(log_level),
      logger_(logger),
      extra_identifier_(extra_identifier),
      context_info_(std::move(context_info))
{
    if (log_level_ >= logger.logger_channel().log_level() && log_level_ != LogLevel::QUIET)
    {
        stream_.emplace();
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

const std::chrono::time_point<std::chrono::system_clock>& Log::time_created() const
{
    return time_created_;
}

bool Log::has_stream() const
{
    return stream_.has_value();
}

std::string Log::str() const
{
    return stream_->str();
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
