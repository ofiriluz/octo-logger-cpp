/**
 * @file manager-config.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/manager-config.hpp"
#include "octo-logger-cpp/sink.hpp"

namespace octo::logger
{
void ManagerConfig::set_option(const ManagerConfig::LoggerOption& option, const std::string& value)
{
    logger_options_[option] = value;
}

void ManagerConfig::set_option(const ManagerConfig::LoggerOption& option, int value)
{
    set_option(option, convert_from<int>(value));
}

void ManagerConfig::set_option(const ManagerConfig::LoggerOption& option, double value)
{
    set_option(option, convert_from<double>(value));
}

void ManagerConfig::set_option(const ManagerConfig::LoggerOption& option, uint8_t value)
{
    set_option(option, convert_from<uint8_t>(value));
}

bool ManagerConfig::has_sink(const std::string& sink_name) const
{
    for (auto& sink : sink_configs_)
    {
        if (sink.sink_name() == sink_name)
        {
            return true;
        }
    }
    return false;
}

const std::vector<SinkConfig>& ManagerConfig::sinks() const
{
    return sink_configs_;
}

const std::vector<SinkPtr>& ManagerConfig::custom_sinks() const
{
    return custom_sinks_;
}

void ManagerConfig::add_sink(const SinkConfig& sink)
{
    sink_configs_.push_back(sink);
}

void ManagerConfig::add_custom_sink(const SinkPtr& sink)
{
    custom_sinks_.push_back(sink);
}

bool ManagerConfig::remove_option(const LoggerOption& option)
{
    if (!has_option(option))
    {
        return false;
    }
    logger_options_.erase(option);
    return true;
}

bool ManagerConfig::has_option(const LoggerOption& option) const
{
    return logger_options_.find(option) != logger_options_.end();
}

bool ManagerConfig::option(const LoggerOption& option, std::string& value) const
{
    if (has_option(option))
    {
        value = logger_options_.at(option);
        return true;
    }
    return false;
}

bool ManagerConfig::option(const LoggerOption& opt, int& value) const
{
    std::string s;
    if (option(opt, s))
    {
        value = convert_to<int>(s);
        return true;
    }
    return false;
}

bool ManagerConfig::option(const LoggerOption& opt, double& value) const
{
    std::string s;
    if (option(opt, s))
    {
        value = convert_to<double>(s);
        return true;
    }
    return false;
}

bool ManagerConfig::option(const LoggerOption& opt, uint8_t& value) const
{
    std::string s;
    if (option(opt, s))
    {
        value = convert_to<uint8_t>(s);
        return true;
    }
    return false;
}

void ManagerConfig::remove_sink(const std::string& sink_name)
{
    for (size_t i = 0; i < sink_configs_.size(); i++)
    {
        if (sink_configs_[i].sink_name() == sink_name)
        {
            sink_configs_.erase(sink_configs_.begin() + i);
            return;
        }
    }
    for (size_t i = 0; i < custom_sinks_.size(); i++)
    {
        if (custom_sinks_[i]->sink_name() == sink_name)
        {
            custom_sinks_.erase(custom_sinks_.begin() + i);
            return;
        }
    }
}
} // namespace octo::logger
