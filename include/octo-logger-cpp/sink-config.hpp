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

#include "octo-logger-cpp/config-utils.hpp"
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

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

        LINE_FORMAT,

#ifndef _WIN32
        FILE_LOG_FILES_PATH,
        FILE_SIZE_PER_LOG_FILE,
        FILE_MAX_LOG_FILES,
        FILE_LOG_FOLDER_PREFIX,
        FILE_SEPARATE_CHANNEL_FILES,
        FILE_COMBINED_CHANNEL_PREFIX,
        FILE_NO_TIME_ON_NAME,
        FILE_NO_DATE_ON_NAME,
        FILE_LOG_FOLDER_NO_SEPARATE_BY_DATE,
        FILE_DISABLE_CONTEXT_INFO,

        SYSLOG_LOG_NAME
#endif
    };

  private:
    std::unordered_map<SinkOption, std::string> options_;
    SinkType sink_type_;
    std::string sink_name_;

  public:
    SinkConfig(std::string sink_name, SinkType sink_type);
    virtual ~SinkConfig() = default;

    template <typename T, typename std::enable_if_t<!std::is_enum_v<T>, bool> = true>
    void set_option(SinkOption option, T value)
    {
        set_option(option, ConfigUtils::convert_from<T>(value));
    }
    template <typename T, typename std::enable_if_t<std::is_enum_v<T>, bool> = true>
    void set_option(SinkOption option, T value)
    {
        set_option(option, static_cast<int>(value));
    }

    template <typename T>
    bool option(SinkOption option, T& value) const
    {
        std::string s;
        if (SinkConfig::option(option, s))
        {
            try
            {
                value = std::move(ConfigUtils::convert_to<T>(s));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        return false;
    }
    template <typename T>
    T option_default(SinkOption option, T default_value) const
    {
        std::string s(option_default(option, ConfigUtils::convert_from<T>(default_value).c_str()));
        try
        {
            T value = ConfigUtils::convert_to<T>(s);
            return value;
        }
        catch (...)
        {
            return default_value;
        }
    }

    bool remove_option(SinkOption option);
    bool has_option(SinkOption option) const;
    std::string const& sink_name() const;
    SinkType sink_type() const;
};

template <>
void SinkConfig::set_option<std::string>(SinkOption option, std::string value);
template <>
bool SinkConfig::option<std::string>(SinkOption option, std::string& value) const;
template <>
char const* SinkConfig::option_default<char const*>(SinkOption option, char const* default_value) const;
} // namespace octo::logger

#endif
