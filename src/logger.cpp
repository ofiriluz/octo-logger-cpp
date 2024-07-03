/**
 * @file logger.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/manager.hpp"

namespace octo::logger
{
void Logger::dump_log(const Log& log) const
{
    Manager::instance().dump(log, channel_view_.channel().channel_name(), context_info_);
}

Logger::Logger(std::string_view channel)
{
    channel_view_ = Manager::instance().create_channel(channel);
}

const Channel& Logger::logger_channel() const
{
    return channel_view_.channel();
}

Channel& Logger::editable_logger_channel()
{
    return channel_view_.editable_channel();
}

Log Logger::trace(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::TRACE, extra_identifier, std::move(context_info), *this);
}

Log Logger::debug(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::DEBUG, extra_identifier, std::move(context_info), *this);
}

Log Logger::info(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::INFO, extra_identifier, std::move(context_info), *this);
}

Log Logger::notice(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::NOTICE, extra_identifier, std::move(context_info), *this);
}

Log Logger::warning(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::WARNING, extra_identifier, std::move(context_info), *this);
}

Log Logger::error(std::string_view extra_identifier, ContextInfo context_info) const
{
    return Log(Log::LogLevel::ERROR, extra_identifier, std::move(context_info), *this);
}

Log Logger::log(Log::LogLevel level, std::string_view extra_identifier, ContextInfo context_info) const
{
    switch (level)
    {
        case Log::LogLevel::QUIET:
            return Log(Log::LogLevel::QUIET, extra_identifier, std::move(context_info), *this);
        case Log::LogLevel::TRACE:
            return trace(extra_identifier, context_info);
        case Log::LogLevel::DEBUG:
            return debug(extra_identifier, context_info);
        case Log::LogLevel::INFO:
            return info(extra_identifier, context_info);
        case Log::LogLevel::NOTICE:
            return notice(extra_identifier, context_info);
        case Log::LogLevel::WARNING:
            return warning(extra_identifier, context_info);
        case Log::LogLevel::ERROR:
            return error(extra_identifier, context_info);
    }
    throw std::runtime_error("No log level");
}

ContextInfo const& Logger::context_info() const
{
    return context_info_;
}

void Logger::add_context_key(ContextInfo::ContextInfoKey key, ContextInfo::ContextInfoValue value)
{

    context_info_.update(key, std::move(value));
}

void Logger::add_context_keys(ContextInfo context_info)
{
    context_info_.update(std::move(context_info));
}

void Logger::remove_context_key(ContextInfo::ContextInfoKey key)
{
    context_info_.erase(key);
}

bool Logger::has_context_key(ContextInfo::ContextInfoKey const& key) const
{
    return context_info_.contains(key);
}

void Logger::clear_context_info()
{
    context_info_.clear();
}

} // namespace octo::logger
