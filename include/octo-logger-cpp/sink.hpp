/**
 * @file sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SINK_HPP_
#define SINK_HPP_

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace octo::logger
{
class Sink
{
  public:
    enum class LineFormat : std::uint8_t
    {
        PLAINTEXT_LONG = 0,  // Used in console
        PLAINTEXT_SHORT = 1, // Used in syslog
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
        JSON = 2,
#endif
    };

  private:
    const SinkConfig config_;
    std::atomic<bool> is_discarding_;

  protected:
    const SinkConfig& config() const;
    const std::string origin_;
    const LineFormat line_format_;

    std::string formatted_log_plaintext_long(Log const& log,
                                             Channel const& channel,
                                             ContextInfo const& context_info,
                                             ContextInfo const& global_context_info,
                                             bool disable_context_info) const;
    std::string formatted_log_plaintext_short(Log const& log, Channel const& channel) const;
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
    std::string formatted_log_json(Log const& log,
                                   Channel const& channel,
                                   ContextInfo const& context_info,
                                   ContextInfo const& global_context_info) const;
#endif

    std::string formatted_log(Log const& log,
                              Channel const& channel,
                              ContextInfo const& context_info,
                              ContextInfo const& global_context_info,
                              bool disable_context_info) const;

    inline static LineFormat extract_format_with_default(const SinkConfig& config, LineFormat default_format)
    {
        return static_cast<LineFormat>(
            config.option_default(SinkConfig::SinkOption::LINE_FORMAT, static_cast<int>(default_format)));
    }

    [[nodiscard]] virtual std::string formatted_context_info(Log const& log,
                                                             Channel const& channel,
                                                             ContextInfo const& context_info,
                                                             ContextInfo const& global_context_info) const;

    inline bool is_discarding() const
    {
        return is_discarding_;
    }
    virtual void stop_impl()
    {
    }

  public:
    explicit Sink(const SinkConfig& config, std::string const& origin, LineFormat format);
    virtual ~Sink() = default;

    virtual void dump(const Log& log,
                      const Channel& channel,
                      ContextInfo const& context_info,
                      ContextInfo const& global_context_info) = 0;
    const std::string& sink_name() const;

    void stop(bool discard);
    virtual void restart_sink() noexcept
    {
    }
};
typedef std::shared_ptr<Sink> SinkPtr;

template <>
void SinkConfig::set_option<Sink::LineFormat>(SinkOption option, Sink::LineFormat value);
} // namespace octo::logger

#endif
