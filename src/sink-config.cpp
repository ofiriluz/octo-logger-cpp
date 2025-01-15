/**
 * @file sink-config.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/sink-config.hpp"

#include "octo-logger-cpp/sink.hpp"
#include "octo-logger-cpp/manager-config.hpp"
#include <utility>

namespace octo::logger
{
SinkConfig::SinkConfig(std::string sink_name, SinkConfig::SinkType sink_type)
    : sink_type_(sink_type), sink_name_(std::move(sink_name))
{
}

bool SinkConfig::remove_option(SinkConfig::SinkOption option)
{
    if (has_option(option))
    {
        options_.erase(option);
        return true;
    }
    return false;
}

bool SinkConfig::has_option(SinkConfig::SinkOption option) const
{
    return options_.find(option) != options_.end();
}

template <>
void SinkConfig::set_option<std::string>(SinkConfig::SinkOption option, std::string value)
{
    options_.insert_or_assign(option, std::move(value));
}

template <>
bool SinkConfig::option(SinkConfig::SinkOption option, std::string& value) const
{
    if (has_option(option))
    {
        value = options_.at(option);
        return true;
    }
    return false;
}

template <>
char const* SinkConfig::option_default(SinkConfig::SinkOption option, char const* default_value) const
{
    if (has_option(option))
    {
        return options_.at(option).c_str();
    }
    return default_value;
}

const std::string& SinkConfig::sink_name() const
{
    return sink_name_;
}

SinkConfig::SinkType SinkConfig::sink_type() const
{
    return sink_type_;
}
} // namespace octo::logger
