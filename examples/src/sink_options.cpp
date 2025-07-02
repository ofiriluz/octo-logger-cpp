/**
 * @file sink_options.cpp
 * @author tuval kohl (tuvalkohl@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-02
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "octo-logger-cpp/manager.hpp"
#include "octo-logger-cpp/logger.hpp"

int main(int argc, char** argv)
{
    auto config = std::make_shared<octo::logger::ManagerConfig>();
    config->set_option(octo::logger::ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL,
                       octo::logger::Log::LogLevel::TRACE);

    // Console sink
    octo::logger::SinkConfig console_sink("Console", octo::logger::SinkConfig::SinkType::CONSOLE_SINK);
    console_sink.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_DISABLE_CONTEXT_INFO, false);
    console_sink.set_option(octo::logger::SinkConfig::SinkOption::LINE_FORMAT,
                            octo::logger::Sink::LineFormat::PLAINTEXT_LONG);
    console_sink.set_option(octo::logger::SinkConfig::SinkOption::USE_SAFE_LOCALTIME_UTC, true);
    config->add_sink(console_sink);

    // Console JSON sink
    octo::logger::SinkConfig console_json_sink("ConsoleJson", octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);
    console_json_sink.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_SERVICE, "cloudwatch");
    console_json_sink.set_option(octo::logger::SinkConfig::SinkOption::USE_SAFE_LOCALTIME_UTC, true);
    config->add_sink(console_json_sink);

    octo::logger::Manager::instance().configure(config);

    octo::logger::Logger logger("Test");
    logger.info() << "HI3";

    octo::logger::Manager::instance().terminate();
}
