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

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include "sink-config.hpp"

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

  private:
    template <class T>
    static T convert_to(const std::string& input)
    {
        std::stringstream convertor;
        convertor << input;
        T out;
        convertor >> out;
        return out;
    }
    template <class T>
    static std::string convert_from(const T& input)
    {
        std::stringstream convertor;
        convertor << input;
        return convertor.str();
    }

  public:
    ManagerConfig() = default;
    virtual ~ManagerConfig() = default;
    void set_option(const LoggerOption& option, const std::string& value);
    void set_option(const LoggerOption& option, int value);
    void set_option(const LoggerOption& option, double value);
    void set_option(const LoggerOption& option, uint8_t value);
    bool has_sink(const std::string& sink_name) const;
    const std::vector<SinkConfig>& sinks() const;
    const std::vector<SinkPtr>& custom_sinks() const;
    void add_sink(const SinkConfig& config);
    void add_custom_sink(const SinkPtr& sink);
    void remove_sink(const std::string& sink_name);
    bool remove_option(const LoggerOption& option);
    bool has_option(const LoggerOption& option) const;
    bool option(const LoggerOption& opt, std::string& value) const;
    bool option(const LoggerOption& opt, int& value) const;
    bool option(const LoggerOption& opt, double& value) const;
    bool option(const LoggerOption& opt, uint8_t& value) const;

    friend class SinkConfig;
};
typedef std::shared_ptr<ManagerConfig> ManagerConfigPtr;
} // namespace octo::logger

#endif
