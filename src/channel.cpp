/**
 * @file channel.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/channel.hpp"

namespace octo::logger
{
Channel::Channel(std::string_view channel_name, Log::LogLevel channel_level)
    : channel_name_(channel_name), channel_level_(channel_level)
{
}

Log::LogLevel Channel::log_level() const
{
    return channel_level_;
}

void Channel::set_log_level(Log::LogLevel channel_level)
{
    channel_level_ = channel_level;
}

const std::string& Channel::channel_name() const
{
    return channel_name_;
}
} // namespace octo::logger
