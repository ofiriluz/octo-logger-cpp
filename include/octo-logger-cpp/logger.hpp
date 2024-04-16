/**
 * @file logger.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/channel-view.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger-test-definitions.hpp"
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace octo::logger
{
class Logger
{
  public:
    typedef std::unordered_map<std::string_view, std::string> ContextInfo;

    static constexpr auto SessionID = "session_id";
    static constexpr auto TenantID = "tenant_id";

  private:
    ContextInfo context_info_;
    ChannelView channel_view_;

  private:
    void dump_log(const Log& log) const;

  public:
    explicit Logger(std::string_view channel);
    virtual ~Logger() = default;

    Log trace(std::string_view extra_identifier = "") const;
    Log debug(std::string_view extra_identifier = "") const;
    Log info(std::string_view extra_identifier = "") const;
    Log notice(std::string_view extra_identifier = "") const;
    Log warning(std::string_view extra_identifier = "") const;
    Log error(std::string_view extra_identifier = "") const;
    Log log(Log::LogLevel level, std::string_view extra_identifier = "") const;

    void add_context_key(std::string_view key, std::string value);
    void add_context_keys(ContextInfo context_info);
    void remove_context_key(std::string_view key);
    bool has_context_key(std::string_view const& key) const;
    void clear_context_info();

    const Channel& logger_channel() const;
    Channel& editable_logger_channel();
    [[nodiscard]] ContextInfo const& context_info() const;

    friend class Log;

    TESTS_MOCK_CLASS(Logger)
};
} // namespace octo::logger

#endif
