/**
 * @file sink-factory.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/sink-factory.hpp"

#include "octo-logger-cpp/sinks/console-json-sink.hpp"
#include "octo-logger-cpp/sinks/console-sink.hpp"
#include "octo-logger-cpp/sinks/file-sink.hpp"
#include "octo-logger-cpp/sinks/syslog-sink.hpp"

namespace octo::logger
{
SinkFactory& SinkFactory::instance()
{
    static SinkFactory factory;
    return factory;
}

SinkPtr SinkFactory::create_sink(const SinkConfig& sink_config)
{
    if (sink_config.sink_type() == SinkConfig::SinkType::CONSOLE_SINK)
    {
        return std::make_shared<ConsoleSink>(sink_config);
    }
#ifndef _WIN32
    if (sink_config.sink_type() == SinkConfig::SinkType::FILE_SINK)
    {
        return std::make_shared<FileSink>(sink_config);
    }
    if (sink_config.sink_type() == SinkConfig::SinkType::SYSLOG_SINK)
    {
        return std::make_shared<SysLogSink>(sink_config);
    }
#endif
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
    if (sink_config.sink_type() == SinkConfig::SinkType::CONSOLE_JSON_SINK)
    {
        return std::make_shared<ConsoleJSONSink>(sink_config);
    }
#endif // OCTO_LOGGER_WITH_JSON_FORMATTING

    return nullptr;
}
} // namespace octo::logger
