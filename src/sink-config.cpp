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

namespace octo::logger
{
SinkConfig::SinkConfig(const std::string& sink_name, const SinkConfig::SinkType& sink_type)
    : sink_name_(sink_name), sink_type_(sink_type)
{
}

void SinkConfig::set_option(const SinkConfig::SinkOption& option, const std::string& value)
{
    options_[option] = value;
}

void SinkConfig::set_option(const SinkConfig::SinkOption& option, int value)
{
    set_option(option, ManagerConfig::convert_from<int>(value));
}

void SinkConfig::set_option(const SinkConfig::SinkOption& option, double value)
{
    set_option(option, ManagerConfig::convert_from<double>(value));
}

void SinkConfig::set_option(const SinkConfig::SinkOption& option, uint8_t value)
{
    set_option(option, ManagerConfig::convert_from<uint8_t>(value));
}

void SinkConfig::set_option(const SinkConfig::SinkOption& option, bool value)
{
    set_option(option, ManagerConfig::convert_from<bool>(value));
}

bool SinkConfig::remove_option(const SinkConfig::SinkOption& option)
{
    if (has_option(option))
    {
        options_.erase(option);
        return true;
    }
    return false;
}

bool SinkConfig::has_option(const SinkConfig::SinkOption& option) const
{
    return options_.find(option) != options_.end();
}

bool SinkConfig::option(const SinkConfig::SinkOption& option, std::string& value) const
{
    if (has_option(option))
    {
        value = options_.at(option);
        return true;
    }
    return false;
}

bool SinkConfig::option(const SinkConfig::SinkOption& opt, int& value) const
{
    std::string s;
    if (option(opt, s))
    {
        try
        {
            value = ManagerConfig::convert_to<int>(s);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    return false;
}

bool SinkConfig::option(const SinkConfig::SinkOption& opt, double& value) const
{
    std::string s;
    if (option(opt, s))
    {
        try
        {
            value = ManagerConfig::convert_to<double>(s);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    return false;
}

bool SinkConfig::option(const SinkConfig::SinkOption& opt, uint8_t& value) const
{
    std::string s;
    if (option(opt, s))
    {
        try
        {
            value = ManagerConfig::convert_to<uint8_t>(s);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    return false;
}

bool SinkConfig::option(const SinkConfig::SinkOption& opt, bool& value) const
{
    std::string s;
    if (option(opt, s))
    {
        try
        {
            value = ManagerConfig::convert_to<bool>(s);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    return false;
}

std::string SinkConfig::option_default(const SinkConfig::SinkOption& option, const std::string& default_value) const
{
    if (has_option(option))
    {
        return options_.at(option);
    }
    return default_value;
}

int SinkConfig::option_default(const SinkConfig::SinkOption& option, int default_value) const
{
    std::string s = option_default(option, ManagerConfig::convert_from<int>(default_value));
    try
    {
        int i = ManagerConfig::convert_to<int>(s);
        return i;
    }
    catch (...)
    {
        return default_value;
    }
}

double SinkConfig::option_default(const SinkConfig::SinkOption& option, double default_value) const
{
    std::string s = option_default(option, ManagerConfig::convert_from<double>(default_value));
    try
    {
        double i = ManagerConfig::convert_to<double>(s);
        return i;
    }
    catch (...)
    {
        return default_value;
    }
}

uint8_t SinkConfig::option_default(const SinkConfig::SinkOption& option, uint8_t default_value) const
{
    std::string s = option_default(option, ManagerConfig::convert_from<uint8_t>(default_value));
    try
    {
        uint8_t i = ManagerConfig::convert_to<uint8_t>(s);
        return i;
    }
    catch (...)
    {
        return default_value;
    }
}

bool SinkConfig::option_default(const SinkConfig::SinkOption& option, bool default_value) const
{
    std::string s = option_default(option, ManagerConfig::convert_from<bool>(default_value));
    try
    {
        bool i = ManagerConfig::convert_to<bool>(s);
        return i;
    }
    catch (...)
    {
        return default_value;
    }
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
