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
#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <iostream>

namespace octo::logger
{
class ConsoleSink : public Sink
{
  private:
    static char constexpr COLOR_RESET[] = "\e[39m";
    static char constexpr COLOR_BLUE[] = "\e[34m";
    static char constexpr COLOR_GREEN[] = "\e[32m";
    static char constexpr COLOR_CYAN[] = "\e[36m";
    static char constexpr COLOR_RED[] = "\e[31m";
    static char constexpr COLOR_YELLOW[] = "\e[33m";

  private:
    bool disable_console_color_;
    bool disable_console_context_info_;

  private:
    static inline void set_color(const char* color)
    {
        std::cout << color;
    }
    static inline void configure_log_color(const Log::LogLevel& log_level)
    {
        switch (log_level)
        {
            case Log::LogLevel::TRACE:
            case Log::LogLevel::DEBUG:
                set_color(COLOR_GREEN);
                break;
            case Log::LogLevel::INFO:
                set_color(COLOR_CYAN);
                break;
            case Log::LogLevel::NOTICE:
                set_color(COLOR_BLUE);
                break;
            case Log::LogLevel::WARNING:
                set_color(COLOR_YELLOW);
                break;
            case Log::LogLevel::ERROR:
                set_color(COLOR_RED);
                break;
            case Log::LogLevel::QUIET:
                break;
        }
    }
    static inline void reset_log_color()
    {
        set_color(COLOR_RESET);
    }

  public:
    explicit ConsoleSink(const SinkConfig& config);
    ~ConsoleSink() override = default;

    void dump(const Log& log,
              const Channel& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override;
};
} // namespace octo::logger

#endif
