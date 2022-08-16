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
#include "octo-logger-cpp/sinks/file-sink.hpp"
#include "octo-logger-cpp/sinks/console-sink.hpp"
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
        return SinkPtr(new ConsoleSink(sink_config));
    }
#ifndef _WIN32
    else if (sink_config.sink_type() == SinkConfig::SinkType::FILE_SINK)
    {
        return SinkPtr(new FileSink(sink_config));
    }
    else if (sink_config.sink_type() == SinkConfig::SinkType::SYSLOG_SINK)
    {
        return SinkPtr(new SysLogSink(sink_config));
    }
#endif
    return SinkPtr();
}
} // namespace octo::logger
