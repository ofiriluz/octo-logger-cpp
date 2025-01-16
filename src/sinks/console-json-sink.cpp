/**
 * @file console-json-sink.cpp
 * @author Denis Sheyer (denis.sheyer@gmail.com)
 * @brief
 * @date 12025-01-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "octo-logger-cpp/sinks/console-json-sink.hpp"

namespace octo::logger
{
ConsoleJSONSink::ConsoleJSONSink(SinkConfig const& sink_config)
    : Sink(sink_config, sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_ORIGIN, ""), LineFormat::JSON),
      host_(sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_HOST, "")),
      service_(sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_SERVICE, ""))
{
}

void ConsoleJSONSink::dump(Log const& log,
                           Channel const& channel,
                           ContextInfo const& context_info,
                           ContextInfo const& global_context_info)
{
    if (log.has_stream())
    {
        auto log_json = construct_log_json(log, channel, context_info, global_context_info);
        log_json["host"] = host_;
        log_json["service"] = service_;
        std::cout << log_json.dump() << std::endl;
    }
}

} // namespace octo::logger
