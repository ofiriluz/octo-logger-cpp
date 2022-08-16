/**
 * @file sink-config.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SINK_CONFIG_HPP_
#define SINK_CONFIG_HPP_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <sstream>
#include <cstdint>

namespace octo::logger
{
class SinkConfig
{
  public:
    enum class SinkType : std::uint8_t
    {
        CONSOLE_SINK,
#ifndef _WIN32
        FILE_SINK,
        SYSLOG_SINK,
#endif
        CUSTOM_SINK
    };

    enum class SinkOption : std::uint8_t
    {
        CONSOLE_DISABLE_COLOR,
        CONSOLE_DISABLE_CONTEXT_INFO,

#ifndef _WIN32
        FILE_LOG_FILES_PATH,
        FILE_SIZE_PER_LOG_FILE,
        FILE_MAX_LOG_FILES,
        FILE_LOG_FOLDER_PREFIX,
        FILE_SEPERATE_CHANNEL_FILES,
        FILE_COMBINED_CHANNEL_PREFIX,
        FILE_NO_TIME_ON_NAME,
        FILE_NO_DATE_ON_NAME,
        FILE_LOG_FOLDER_NO_SEPERATE_BY_DATE,
        FILE_DISABLE_CONTEXT_INFO,

        SYSLOG_LOG_NAME
#endif
    };

  private:
    std::map<SinkOption, std::string> options_;
    SinkType sink_type_;
    std::string sink_name_;

  public:
    SinkConfig(const std::string& sink_name, const SinkType& sink_type);
    virtual ~SinkConfig() = default;

    void set_option(const SinkOption& option, const std::string& value);
    void set_option(const SinkOption& option, int value);
    void set_option(const SinkOption& option, double value);
    void set_option(const SinkOption& option, uint8_t value);
    void set_option(const SinkOption& option, bool value);
    bool remove_option(const SinkOption& option);
    bool has_option(const SinkOption& option) const;
    bool option(const SinkOption& option, std::string& value) const;
    bool option(const SinkOption& opt, int& value) const;
    bool option(const SinkOption& opt, double& value) const;
    bool option(const SinkOption& opt, uint8_t& value) const;
    bool option(const SinkOption& opt, bool& value) const;
    std::string option_default(const SinkOption& option, const std::string& default_value) const;
    int option_default(const SinkOption& option, int default_value) const;
    double option_default(const SinkOption& option, double default_value) const;
    uint8_t option_default(const SinkOption& option, uint8_t default_value) const;
    bool option_default(const SinkOption& option, bool default_value) const;
    const std::string& sink_name() const;
    SinkType sink_type() const;
};
} // namespace octo::logger

#endif
