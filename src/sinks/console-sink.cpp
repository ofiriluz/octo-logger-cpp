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

namespace octo::logger
{

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
