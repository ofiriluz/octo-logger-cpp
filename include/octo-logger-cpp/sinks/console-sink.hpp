/**
 * @file console-sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef CONSOLE_SINK_HPP_
#define CONSOLE_SINK_HPP_

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace octo::logger
{
class ConsoleSink : public Sink
{
  private:
    bool disable_console_color_;
    bool disable_console_context_info_;

  private:
    void set_color(const char* color);
    void configure_log_color(const Log::LogLevel& log_level);
    void reset_log_color();

  public:
    explicit ConsoleSink(const SinkConfig& config);
    ~ConsoleSink() override = default;

    void dump(const Log& log, const Channel& channel, Logger::ContextInfo const& context_info) override;
};
} // namespace octo::logger

#endif
