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

#include <nlohmann/json.hpp>

namespace octo::logger
{
ConsoleJSONSink::ConsoleJSONSink(SinkConfig const& sink_config)
    : Sink(sink_config,
           sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_ORIGIN, DEFAULT_ORIGIN),
           LineFormat::JSON),
      host_(sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_HOST, DEFAULT_HOST)),
      service_(sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_SERVICE, DEFAULT_SERVICE)),
      indent_(sink_config.option_default(SinkConfig::SinkOption::CONSOLE_JSON_INDENT, DEFAULT_INDENT))
{
}

void ConsoleJSONSink::dump(Log const& log,
                           Channel const& channel,
                           ContextInfo const& context_info,
                           ContextInfo const& global_context_info)
{
    if (log.has_stream())
    {
        try
        {
            nlohmann::json log_json(construct_log_json(log, channel, context_info, global_context_info));
            log_json["host"] = host_;
            log_json["service"] = service_;
            std::cout << log_json.dump(indent_) << std::endl;
        }
        catch (nlohmann::json::exception const& ex)
        {
            // Fallback to default upon exception
            std::cerr << "Failed to dump log to console in JSON format: " << ex.what() << std::endl;
            std::cout << formatted_log_plaintext_long(log, channel, context_info, global_context_info, false)
                      << std::endl;
        }
    }
}

} // namespace octo::logger
