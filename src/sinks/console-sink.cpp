/**
 * @file console-sink.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/sinks/console-sink.hpp"

namespace
{
static constexpr const char COLOR_RESET[] = "\e[39m";
static constexpr const char COLOR_BLUE[] = "\e[34m";
static constexpr const char COLOR_GREEN[] = "\e[32m";
static constexpr const char COLOR_CYAN[] = "\e[36m";
static constexpr const char COLOR_RED[] = "\e[31m";
static constexpr const char COLOR_YELLOW[] = "\e[33m";
} // namespace

namespace octo::logger
{
void ConsoleSink::set_color(const char* color)
{
    std::cout << color;
}

void ConsoleSink::configure_log_color(const Log::LogLevel& log_level)
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

void ConsoleSink::reset_log_color()
{
    set_color(COLOR_RESET);
}

ConsoleSink::ConsoleSink(const SinkConfig& config)
    : Sink(config, "", extract_format_with_default(config, LineFormat::PLAINTEXT_LONG))
{
    disable_console_color_ = Sink::config().option_default(SinkConfig::SinkOption::CONSOLE_DISABLE_COLOR, false);
    disable_console_context_info_ =
        Sink::config().option_default(SinkConfig::SinkOption::CONSOLE_DISABLE_CONTEXT_INFO, true);
}

void ConsoleSink::dump(const Log& log,
                       const Channel& channel,
                       ContextInfo const& context_info,
                       ContextInfo const& global_context_info)
{
    if (log.has_stream())
    {
        if (!disable_console_color_)
        {
            configure_log_color(log.log_level());
        }
        std::cout << formatted_log(log, channel, context_info, global_context_info, disable_console_context_info_);

        if (!disable_console_color_)
        {
            reset_log_color();
        }
        std::cout << std::endl;
    }
}
} // namespace octo::logger
