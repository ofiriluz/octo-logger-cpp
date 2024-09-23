#ifdef OCTO_LOGGER_WITH_AWS

#include "cloudwatch-sink-mock.hpp"
#include "catch2-matchers.hpp"
#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/manager.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace
{
using octo::logger::unittests::CloudWatchSinkMock;
using octo::logger::unittests::JSONEquals;
using octo::logger::unittests::LogStreamTypeEquals;
using SinkConfig = octo::logger::SinkConfig;
using Log = octo::logger::Log;
using Logger = octo::logger::Logger;
using Channel = octo::logger::Channel;
using ContextInfoInitializerList = octo::logger::ContextInfo::ContextInfoInitializerList;

class CloudWatchSinkTestsFixture
{
  public:
    typedef std::vector<std::pair<SinkConfig::SinkOption, std::variant<bool, double, int, std::string, uint8_t>>>
        ConfigOptionsVector;

  public:
    octo::logger::Manager& manager_;
    Logger logger_;

  public:
    CloudWatchSinkTestsFixture() : manager_(octo::logger::Manager::instance()), logger_("test_logger")
    {
    }

    ~CloudWatchSinkTestsFixture()
    {
        octo::logger::Manager::reset_manager();
    }

    static SinkConfig get_sink_config(std::string_view sink_name)
    {
        return {sink_name.data(), SinkConfig::SinkType::CUSTOM_SINK};
    }

    Log get_log(Log::LogLevel const& log_level, std::string_view extra_identifier) const
    {
        return logger_.log(log_level, extra_identifier);
    }

    Channel get_channel(std::string const& channel_name)
    {
        manager_.create_channel(channel_name);
        return manager_.channel(channel_name);
    }

    static void set_sink_config_options(SinkConfig& sink_config, ConfigOptionsVector const& options)
    {
        for (auto const& itr : options)
        {
            if (std::holds_alternative<bool>(itr.second))
            {
                sink_config.set_option(itr.first, std::get<bool>(itr.second));
            }
            else if (std::holds_alternative<double>(itr.second))
            {
                sink_config.set_option(itr.first, std::get<double>(itr.second));
            }
            else if (std::holds_alternative<int>(itr.second))
            {
                sink_config.set_option(itr.first, std::get<int>(itr.second));
            }
            else if (std::holds_alternative<std::string>(itr.second))
            {
                sink_config.set_option(itr.first, std::get<std::string>(itr.second));
            }
            else if (std::holds_alternative<uint8_t>(itr.second))
            {
                sink_config.set_option(itr.first, std::get<uint8_t>(itr.second));
            }
        }
    }
};

} // namespace

TEST_CASE_METHOD(CloudWatchSinkTestsFixture, "CloudWatchSink Initialization Tests", "[cloudwatch_sink]")
{
    SECTION("Initialize")
    {
        SinkConfig const sink_config(get_sink_config("InitializationSinkConfig"));
        CloudWatchSinkMock const sink_by_channel(
            sink_config, "test_origin", CloudWatchSinkMock::LogStreamType::BY_CHANNEL, true, "test_group_name");
        REQUIRE_THAT(sink_by_channel.origin_getter(), Catch::Matchers::Equals("test_origin"));
        REQUIRE_THAT(sink_by_channel.log_stream_type_getter(),
                     LogStreamTypeEquals(CloudWatchSinkMock::LogStreamType::BY_CHANNEL));
        REQUIRE(sink_by_channel.include_date_on_log_stream_getter());
        REQUIRE_THAT(sink_by_channel.log_group_name_getter(), Catch::Matchers::Equals("test_group_name"));

        CloudWatchSinkMock const sink_by_extra_id(
            sink_config, "test_origin", CloudWatchSinkMock::LogStreamType::BY_EXTRA_ID, false, "test_group_name");
        REQUIRE_THAT(sink_by_extra_id.origin_getter(), Catch::Matchers::Equals("test_origin"));
        REQUIRE_THAT(sink_by_extra_id.log_stream_type_getter(),
                     LogStreamTypeEquals(CloudWatchSinkMock::LogStreamType::BY_EXTRA_ID));
        REQUIRE_FALSE(sink_by_extra_id.include_date_on_log_stream_getter());
        REQUIRE_THAT(sink_by_extra_id.log_group_name_getter(), Catch::Matchers::Equals("test_group_name"));

        CloudWatchSinkMock const sink_by_both(
            sink_config, "test_origin", CloudWatchSinkMock::LogStreamType::BY_BOTH, true, "test_group_name");
        REQUIRE_THAT(sink_by_both.origin_getter(), Catch::Matchers::Equals("test_origin"));
        REQUIRE_THAT(sink_by_both.log_stream_type_getter(),
                     LogStreamTypeEquals(CloudWatchSinkMock::LogStreamType::BY_BOTH));
        REQUIRE(sink_by_both.include_date_on_log_stream_getter());
        REQUIRE_THAT(sink_by_both.log_group_name_getter(), Catch::Matchers::Equals("test_group_name"));
    }
}

TEST_CASE_METHOD(CloudWatchSinkTestsFixture, "CloudWatchSink InitContextInfo Tests", "[cloudwatch_sink]")
{
    SinkConfig const sink_config(get_sink_config("TestSinkConfig"));
    CloudWatchSinkMock const sink(sink_config, "test_origin");
    Channel const channel(get_channel("test_channel"));

    SECTION("Init Basic Info")
    {
        Log const test_log(get_log(Log::LogLevel::QUIET, "408aa391-a07e-4692-a02b-a17e8950fa24"));
        auto const context_info_json(sink.init_context_info_wrapper(test_log, channel, {}, {}));
        nlohmann::json const expected_json({
            {"session_id", "408aa391-a07e-4692-a02b-a17e8950fa24"},
        });
        REQUIRE_THAT(context_info_json, JSONEquals(expected_json));
    }

    SECTION("Init Empty Object")
    {
        Log const test_log(get_log(Log::LogLevel::QUIET, ""));
        nlohmann::json dst(nlohmann::json::value_t::null);
        REQUIRE_NOTHROW(sink.init_context_info_wrapper(dst, test_log, channel, {}, {}));
        REQUIRE(dst.is_object());
        REQUIRE(dst.empty());
    }

    SECTION("Init With ContextInfo")
    {
        struct TestData
        {
            std::string const session_id;
            nlohmann::json dst;
            octo::logger::ContextInfo const context_info;
            nlohmann::json const expected_result;
        };
        std::vector<TestData> test_data{
            {"52c1fdd2-5987-49e9-8e30-6fbaf08b40dc",
             nlohmann::json::value_t::null,
             {{"property_1", "property 1 value"}, {"property_2", "property 2 value"}},
             {{"session_id", "52c1fdd2-5987-49e9-8e30-6fbaf08b40dc"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test merge dst with ContextInfo.
             "3c261af5-db38-4b5f-81d8-9f7cb9003fee",
             {{"property_1", "property 1 value"}},
             {{"property_2", "property 2 value"}, {"property_3", "property 3 value"}},
             {{"session_id", "3c261af5-db38-4b5f-81d8-9f7cb9003fee"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"},
              {"property_3", "property 3 value"}}},
            {// Test keep dst prop over ContextInfo prop.
             "79f716c4-6dba-4018-a298-dc96fb3ccfb2",
             {{"property_1", "property 1 value"}},
             {{"property_1", "different property 1 value which should be ignored"}, {"property_2", "property 2 value"}},
             {{"session_id", "79f716c4-6dba-4018-a298-dc96fb3ccfb2"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test get SessionID from ContextInfo.
             "",
             {{"property_1", "property 1 value"}},
             {{"session_id", "69426e8b-83a8-47f1-9e09-87857819db75"}, {"property_2", "property 2 value"}},
             {{"session_id", "69426e8b-83a8-47f1-9e09-87857819db75"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test get SessionID from ContextInfo even if empty.
             "",
             {{"property_1", "property 1 value"}},
             {{"session_id", ""}, {"property_2", "property 2 value"}},
             {{"session_id", ""}, {"property_1", "property 1 value"}, {"property_2", "property 2 value"}}},
            {// Test keep SessionID from dst over ContextInfo.
             "",
             {{"session_id", "6101ce70-2b03-4d0d-9b4e-7bf646da14b7"}, {"property_1", "property 1 value"}},
             {{"session_id", "b89354d6-7784-4917-a5a0-10dddd27d6de"}, {"property_2", "property 2 value"}},
             {{"session_id", "6101ce70-2b03-4d0d-9b4e-7bf646da14b7"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test keep SessionID from dst over ContextInfo even if empty.
             "",
             {{"session_id", ""}, {"property_1", "property 1 value"}},
             {{"session_id", "b89354d6-7784-4917-a5a0-10dddd27d6de"}, {"property_2", "property 2 value"}},
             {{"session_id", ""}, {"property_1", "property 1 value"}, {"property_2", "property 2 value"}}},
            {// Test get SessionID from ExtraIdentifier.
             "8a69e3db-5f3e-4356-92de-ded1c5ec4424",
             {{"property_1", "property 1 value"}},
             {{"property_2", "property 2 value"}},
             {{"session_id", "8a69e3db-5f3e-4356-92de-ded1c5ec4424"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test get SessionID from ExtraIdentifier over dst.
             "fb949831-09ca-4016-b455-5a2a1fa65e22",
             {{"session_id", "33ebf6ff-7765-4fec-8100-d25cc6eb613b"}, {"property_1", "property 1 value"}},
             {{"property_2", "property 2 value"}},
             {{"session_id", "fb949831-09ca-4016-b455-5a2a1fa65e22"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test get SessionID from ExtraIdentifier over ContextInfo.
             "70205245-99bd-458d-b27a-59c41db99665",
             {{"session_id", ""}, {"property_1", "property 1 value"}},
             {{"session_id", "c8f3e777-dcbc-4522-b63d-ec7635e30eb3"}, {"property_2", "property 2 value"}},
             {{"session_id", "70205245-99bd-458d-b27a-59c41db99665"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test get SessionID from ExtraIdentifier dst and ContextInfo.
             "a6512280-a15e-414c-a937-10e9d248b997",
             {{"session_id", "c768bd36-ea3c-405c-bc34-8ef52dc96b54"}, {"property_1", "property 1 value"}},
             {{"session_id", "b5d84d1c-c55b-4024-8e87-0f263d51aa54"}, {"property_2", "property 2 value"}},
             {{"session_id", "a6512280-a15e-414c-a937-10e9d248b997"},
              {"property_1", "property 1 value"},
              {"property_2", "property 2 value"}}},
            {// Test no SessionID.
             "",
             {{"property_1", "property 1 value"}},
             {{"property_2", "property 2 value"}},
             {{"property_1", "property 1 value"}, {"property_2", "property 2 value"}}},
        };

        for (auto& itr_context_info : test_data)
        {
            CAPTURE(itr_context_info.session_id, itr_context_info.dst, itr_context_info.context_info);

            Log const test_log(get_log(Log::LogLevel::QUIET, itr_context_info.session_id));

            REQUIRE_NOTHROW(sink.init_context_info_wrapper(
                itr_context_info.dst, test_log, channel, itr_context_info.context_info, {}));
            REQUIRE_THAT(itr_context_info.dst, JSONEquals(itr_context_info.expected_result));
        }
    }

    SECTION("Wrong Type")
    {
        Log const test_log(get_log(Log::LogLevel::QUIET, ""));

        std::string const base_error("Wrong context_info destination type ");
        struct TestData
        {
            nlohmann::json dst;
            std::string const expected_error;
        };
        std::vector<TestData> test_data{
            {nlohmann::json::value_t::array, base_error + "array"},
            {nlohmann::json::value_t::string, base_error + "string"},
            {nlohmann::json::value_t::boolean, base_error + "boolean"},
            {nlohmann::json::value_t::number_integer, base_error + "number"},
            {nlohmann::json::value_t::number_unsigned, base_error + "number"},
            {nlohmann::json::value_t::number_float, base_error + "number"},
            {nlohmann::json::value_t::binary, base_error + "binary"},
            {nlohmann::json::value_t::discarded, base_error + "discarded"},
        };

        for (auto& itr_context_info : test_data)
        {
            CAPTURE(itr_context_info.dst.type_name());

            REQUIRE_THROWS_WITH(sink.init_context_info_wrapper(itr_context_info.dst, test_log, channel, {}, {}),
                                Catch::Matchers::Equals(itr_context_info.expected_error));
        }
    }
}

#endif
