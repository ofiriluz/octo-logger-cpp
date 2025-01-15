/**
 * @file manager-config.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MANAGER_CONFIG_HPP_
#define MANAGER_CONFIG_HPP_

#include "octo-logger-cpp/config-utils.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace octo::logger
{
class Sink;
typedef std::shared_ptr<Sink> SinkPtr;
class ManagerConfig
{
  public:
    enum class LoggerOption : std::uint8_t
    {
        DEFAULT_CHANNEL_LEVEL
    };

  private:
    std::vector<SinkConfig> sink_configs_;
    std::vector<SinkPtr> custom_sinks_;
    std::map<LoggerOption, std::string> logger_options_;

  public:
    ManagerConfig() = default;
    virtual ~ManagerConfig() = default;

    template <typename T, typename std::enable_if_t<!std::is_enum_v<T>, bool> = true>
    void set_option(LoggerOption option, T value)
    {
        set_option(option, ConfigUtils::convert_from<T>(value));
    }
    template <typename T, typename std::enable_if_t<std::is_enum_v<T>, bool> = true>
    void set_option(LoggerOption option, T value)
    {
        set_option(option, static_cast<int>(value));
    }
    template <typename T>
    bool option(LoggerOption option, T& value) const
    {
        std::string s;
        if (ManagerConfig::option(option, s))
        {
            value = ConfigUtils::convert_to<int>(s);
            return true;
        }
        return false;
    }

    bool has_sink(const std::string& sink_name) const;
    const std::vector<SinkConfig>& sinks() const;
    const std::vector<SinkPtr>& custom_sinks() const;
    void add_sink(const SinkConfig& config);
    void add_custom_sink(const SinkPtr& sink);
    void remove_sink(const std::string& sink_name);
    bool remove_option(const LoggerOption& option);
    bool has_option(const LoggerOption& option) const;

    friend class SinkConfig;
};
typedef std::shared_ptr<ManagerConfig> ManagerConfigPtr;

typedef std::shared_ptr<ManagerConfig> ManagerConfigPtr;
template <>
void ManagerConfig::set_option<std::string>(LoggerOption option, std::string value);
template <>
bool ManagerConfig::option<std::string>(LoggerOption option, std::string& value) const;
} // namespace octo::logger

#endif
