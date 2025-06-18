/**
 * @file console-json-sink-tests.cpp
 * @author Denis Sheyer (denis.sheyer@gmail.com)
 * @brief
 * @date 12025-01-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <catch2/catch_all.hpp>

#include "catch2-matchers.hpp"
#include "log-mock.hpp"
#include "logger-mock.hpp"
#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/log-level.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sinks/console-json-sink.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#define CAPTURE_STDOUT(__dst, ...)                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        std::ostringstream captured;                                                                                   \
        std::streambuf* old_buff = std::cout.rdbuf(captured.rdbuf());                                                  \
        __VA_ARGS__;                                                                                                   \
        std::cout.rdbuf(old_buff);                                                                                     \
        __dst = captured.str();                                                                                        \
    } while (false)

TEST_CASE("Test no Log stream", "[console-json-sink]")
{
    std::vector<octo::logger::LogLevel> const test_data{
        octo::logger::LogLevel::TRACE,
        octo::logger::LogLevel::DEBUG,
        octo::logger::LogLevel::INFO,
        octo::logger::LogLevel::NOTICE,
        octo::logger::LogLevel::WARNING,
        octo::logger::LogLevel::ERROR,
        octo::logger::LogLevel::QUIET,
    };
    for (auto const& log_level : test_data)
    {
        DYNAMIC_SECTION(octo::logger::LogLevelUtils::level_to_string(log_level))
        {
            octo::logger::unittests::LoggerMock logger_mock;
            logger_mock.channel_view_getter().editable_channel().set_log_level(octo::logger::LogLevel::QUIET);
            octo::logger::unittests::LogMock const log_mock(
                log_level, octo::logger::LogLevelUtils::level_to_string(log_level), {}, logger_mock);
            octo::logger::SinkConfig sink_config("console_json_sink",
                                                 octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);
            octo::logger::ConsoleJSONSink sink(sink_config);
            octo::logger::Channel channel("test_channel", log_level);
            std::string log_stdout;
            CAPTURE_STDOUT(log_stdout, sink.dump(log_mock, logger_mock.logger_channel(), {}, {}));
            REQUIRE(log_stdout.empty());
        }
    }
}

TEST_CASE("Test default config", "[console-json-sink]")
{
    std::vector<octo::logger::LogLevel> const test_data{
        octo::logger::LogLevel::TRACE,
        octo::logger::LogLevel::DEBUG,
        octo::logger::LogLevel::INFO,
        octo::logger::LogLevel::NOTICE,
        octo::logger::LogLevel::WARNING,
        octo::logger::LogLevel::ERROR,
    };
    for (auto const& log_level : test_data)
    {
        DYNAMIC_SECTION(octo::logger::LogLevelUtils::level_to_string(log_level))
        {
            octo::logger::unittests::LoggerMock logger_mock;
            logger_mock.channel_view_getter().editable_channel().set_log_level(log_level);
            octo::logger::unittests::LogMock const log_mock(
                log_level, octo::logger::LogLevelUtils::level_to_string(log_level), {}, logger_mock);
            octo::logger::SinkConfig sink_config("console_json_sink",
                                                 octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);
            octo::logger::ConsoleJSONSink sink(sink_config);
            octo::logger::Channel channel("test_channel", log_level);
            std::string log_stdout;
            CAPTURE_STDOUT(log_stdout, sink.dump(log_mock, logger_mock.logger_channel(), {}, {}));
            REQUIRE_FALSE(log_stdout.empty());

            nlohmann::json log_json(nlohmann::json::parse(log_stdout));
            REQUIRE(log_json.is_object());
            REQUIRE(log_json.contains("host"));
            REQUIRE(log_json["host"].is_string());
            REQUIRE(log_json["host"].get<std::string>().empty());
            REQUIRE(log_json.contains("service"));
            REQUIRE(log_json["service"].is_string());
            REQUIRE(log_json["service"].get<std::string>().empty());
            REQUIRE(log_json.contains("origin"));
            REQUIRE(log_json["origin"].is_string());
            REQUIRE(log_json["origin"].get<std::string>().empty());

            REQUIRE(log_json.contains("log_level"));
            REQUIRE(log_json["log_level"].is_string());
            REQUIRE_THAT(log_json["log_level"].get<std::string>(),
                         Catch::Matchers::Equals(octo::logger::LogLevelUtils::level_to_string_upper(log_level)));
        }
    }
}

TEST_CASE("Test with properties", "[console-json-sink]")
{
    struct TestData
    {
        octo::logger::LogLevel const log_level;
        std::string const host;
        std::string const service;
        std::string const origin;
        octo::logger::ContextInfo const context_info;
        octo::logger::ContextInfo const global_context_info;
    };
    std::vector<TestData> const test_data{
        {octo::logger::LogLevel::TRACE, "", "", "", {}, {}},
        {octo::logger::LogLevel::DEBUG, "host1", "", "", {}, {}},
        {octo::logger::LogLevel::INFO, "", "service2", "", {}, {}},
        {octo::logger::LogLevel::NOTICE, "", "", "origin3", {}, {}},
        {octo::logger::LogLevel::WARNING, "host4", "service4", "", {}, {}},
        {octo::logger::LogLevel::ERROR, "", "service5", "origin5", {}, {}},
        {octo::logger::LogLevel::INFO, "host6", "service6", "origin6", {}, {}},
        {octo::logger::LogLevel::INFO,
         "host7",
         "service7",
         "origin7",
         {{"key_a", "value_a"}, {"key_b", "value_b"}},
         {}},
        {octo::logger::LogLevel::INFO,
         "host8",
         "service8",
         "origin8",
         {},
         {{"global_key_a", "global_value_a"}, {"global_key_b", "global_value_b"}}},
        {octo::logger::LogLevel::INFO,
         "host9",
         "service9",
         "origin9",
         {{"key_a", "value_a"}, {"key_b", "value_b"}},
         {{"global_key_a", "global_value_a"}, {"global_key_b", "global_value_b"}}},
    };
    std::size_t idx = 0; // Running index to force DYNAMIC_SECTION to be unique.
    for (auto const& [log_level, host, service, origin, context_info, global_context_info] : test_data)
    {
        DYNAMIC_SECTION(idx++)
        {
            octo::logger::unittests::LoggerMock logger_mock;
            logger_mock.channel_view_getter().editable_channel().set_log_level(log_level);
            octo::logger::unittests::LogMock const log_mock(
                log_level, octo::logger::LogLevelUtils::level_to_string(log_level), {}, logger_mock);
            octo::logger::SinkConfig sink_config("console_json_sink",
                                                 octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);

            sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_HOST, host);
            sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_SERVICE, service);
            sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_ORIGIN, origin);

            octo::logger::ConsoleJSONSink sink(sink_config);
            octo::logger::Channel channel("test_channel", log_level);
            std::string log_stdout;
            CAPTURE_STDOUT(log_stdout,
                           sink.dump(log_mock, logger_mock.logger_channel(), context_info, global_context_info));
            REQUIRE_FALSE(log_stdout.empty());

            nlohmann::json log_json(nlohmann::json::parse(log_stdout));
            REQUIRE(log_json.is_object());
            REQUIRE(log_json.contains("host"));
            REQUIRE(log_json["host"].is_string());
            REQUIRE_THAT(log_json["host"].get<std::string>(), Catch::Matchers::Equals(host));
            REQUIRE(log_json.contains("service"));
            REQUIRE(log_json["service"].is_string());
            REQUIRE_THAT(log_json["service"].get<std::string>(), Catch::Matchers::Equals(service));
            REQUIRE(log_json.contains("origin"));
            REQUIRE(log_json["origin"].is_string());
            REQUIRE_THAT(log_json["origin"].get<std::string>(), Catch::Matchers::Equals(origin));

            REQUIRE(log_json.contains("log_level"));
            REQUIRE(log_json["log_level"].is_string());
            REQUIRE_THAT(log_json["log_level"].get<std::string>(),
                         Catch::Matchers::Equals(octo::logger::LogLevelUtils::level_to_string_upper(log_level)));

            REQUIRE(log_json.contains("context_info"));
            REQUIRE(log_json["context_info"].is_object());
            for (auto const& [key, value] : context_info)
            {
                CAPTURE(key.data());
                REQUIRE(log_json["context_info"].contains(key));
                REQUIRE(log_json["context_info"][key].is_string());
                REQUIRE_THAT(log_json["context_info"][key].get<std::string>(), Catch::Matchers::Equals(value));
            }
            for (auto const& [key, value] : global_context_info)
            {
                if (context_info.contains(key))
                {
                    // Local context info has priority over global context info.
                    continue;
                }
                CAPTURE(key.data());
                REQUIRE(log_json["context_info"].contains(key));
                REQUIRE(log_json["context_info"][key].is_string());
                REQUIRE_THAT(log_json["context_info"][key].get<std::string>(), Catch::Matchers::Equals(value));
            }
            for (auto const& [key, value] :
                 log_json["context_info"].get<std::unordered_map<std::string, std::string>>())
            {
                // Check context info keys are either in context_info or global_context_info or "session_id" which is
                // handled separately.
                CAPTURE(key);
                bool const found =
                    context_info.contains(key) || global_context_info.contains(key) || key == "session_id";
                REQUIRE(found);
            }
        }
    }
}

TEST_CASE("Test log_thread_id configuration adds thread id to context_info", "[console-json-sink][thread-id]")
{
    octo::logger::LogLevel log_level = octo::logger::LogLevel::INFO;
    std::string host = "host_thread";
    std::string service = "service_thread";
    std::string origin = "origin_thread";
    octo::logger::ContextInfo context_info = {{"custom_key", "custom_value"}};
    octo::logger::ContextInfo global_context_info = {{"global_key", "global_value"}};

    octo::logger::unittests::LoggerMock logger_mock;
    logger_mock.channel_view_getter().editable_channel().set_log_level(log_level);
    octo::logger::unittests::LogMock const log_mock(
        log_level, octo::logger::LogLevelUtils::level_to_string(log_level), {}, logger_mock);

    octo::logger::SinkConfig sink_config("console_json_sink",
                                         octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);

    sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_HOST, host);
    sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_SERVICE, service);
    sink_config.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_ORIGIN, origin);
    sink_config.set_option(octo::logger::SinkConfig::SinkOption::LOG_THREAD_ID, true);

    octo::logger::ConsoleJSONSink sink(sink_config);
    std::string log_stdout;
    CAPTURE_STDOUT(log_stdout,
        sink.dump(log_mock, logger_mock.logger_channel(), context_info, global_context_info));
    REQUIRE_FALSE(log_stdout.empty());

    nlohmann::json log_json(nlohmann::json::parse(log_stdout));
    REQUIRE(log_json.is_object());
    REQUIRE(log_json.contains("context_info"));
    REQUIRE(log_json["context_info"].is_object());

    // Should contain the thread_id key
    REQUIRE(log_json["context_info"].contains("thread_id"));
    REQUIRE(log_json["context_info"]["thread_id"].is_string());
    // Should not be empty
    REQUIRE_FALSE(log_json["context_info"]["thread_id"].get<std::string>().empty());

    // Should still contain the other context info keys
    REQUIRE(log_json["context_info"].contains("custom_key"));
    REQUIRE(log_json["context_info"]["custom_key"].get<std::string>() == "custom_value");
    REQUIRE(log_json["context_info"].contains("global_key"));
    REQUIRE(log_json["context_info"]["global_key"].get<std::string>() == "global_value");
}
