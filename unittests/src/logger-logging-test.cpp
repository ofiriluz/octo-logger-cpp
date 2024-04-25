#include "octo-logger-cpp/logger.hpp"
#include "catch2-matchers.hpp"
#include "octo-logger-cpp/manager.hpp"
#include "logger-mock.hpp"
#include "dummy-sink.hpp"
#include <catch2/catch_all.hpp>
#include <string>
#include <unordered_map>

namespace
{
using octo::logger::unittests::ContextInfoEquals;
using octo::logger::unittests::LoggerMock;

class LoggerLoggingTestsFixture
{
  public:
    LoggerLoggingTestsFixture()
    {
        octo::logger::ManagerConfigPtr manager_config = std::make_shared<octo::logger::ManagerConfig>();
        manager_config->add_custom_sink(std::make_shared<octo::logger::DummySink>());
        octo::logger::Manager::instance().configure(manager_config);
    }
    ~LoggerLoggingTestsFixture()
    {
        octo::logger::Manager::reset_manager();
    }
};

} // namespace

TEST_CASE_METHOD(LoggerTestsFixture, "Logger Initialization Tests", "[logger]")
{
    LoggerMock logger("basic_test_logger");

    SECTION("Miscellaneous")
    {
        REQUIRE_THAT(logger.channel_name_getter(), Catch::Matchers::Equals("basic_test_logger"));
        REQUIRE(logger.context_info().empty());
    }

    SECTION("Channel sharing upon same name")
    {
        LoggerMock logger2("basic_test_logger");

        REQUIRE(&logger.logger_channel() == &logger2.logger_channel());
    }

}



TEST_CASE_METHOD(LoggerTestsFixture, "Logger ContextInfo Tests", "[logger]")
{
    LoggerMock logger;

    SECTION("Add to ContextInfo")
    {
        REQUIRE(logger.context_info().empty());
        logger.add_context_key("test_key_1", "test_value_1");
        REQUIRE_FALSE(logger.context_info().empty());

        REQUIRE_THAT(logger.context_info(),
                     ContextInfoEquals(LoggerMock::ContextInfo({{"test_key_1", "test_value_1"}})));

        logger.add_context_key("test_key_2", "test_value_2");
        REQUIRE_THAT(logger.context_info(),
                     ContextInfoEquals(
                         LoggerMock::ContextInfo({{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}})));

        SECTION("Add Duplicate")
        {
            logger.add_context_key("test_key_3", "test_value_3");
            REQUIRE_THAT(
                logger.context_info(),
                ContextInfoEquals(LoggerMock::ContextInfo(
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"test_key_3", "test_value_3"}})));
            logger.add_context_key("test_key_3", "test_value_3");
            REQUIRE_THAT(
                logger.context_info(),
                ContextInfoEquals(LoggerMock::ContextInfo(
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"test_key_3", "test_value_3"}})));
        }

        SECTION("Overwrite Value")
        {
            logger.clear_context_info();

            logger.add_context_key("test_key_1", "test_value_1");
            REQUIRE_THAT(logger.context_info(),
                         ContextInfoEquals(LoggerMock::ContextInfo({{"test_key_1", "test_value_1"}})));
            logger.add_context_key("test_key_1", "test_value_999");
            REQUIRE_THAT(logger.context_info(),
                         ContextInfoEquals(LoggerMock::ContextInfo({{"test_key_1", "test_value_999"}})));
        }

        SECTION("Add Bulk")
        {
            struct TestData
            {
                std::string const test_id;
                LoggerMock::ContextInfo const context_info;
                LoggerMock::ContextInfo const existing_context_info;
                LoggerMock::ContextInfo const expected_result;
            };
            std::vector<TestData> test_data{
                {
                    // Add empty ContextInfo.
                    "c68df3e0-4c56-45f9-bf27-1652900581fc",
                    {},
                    {},
                    {},
                },
                {
                    // Merge empty ContextInfo.
                    "e830f321-5250-4b75-ae79-f6bacc202369",
                    {},
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
                },
                {
                    // Empty merge ContextInfo.
                    "7bf70e77-cf71-4542-86b1-f5823f6023f1",
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
                    {},
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
                },
                {
                    // Merge ContextInfo.
                    "84ed381a-968a-4712-8cac-ffe1e86d3bfd",
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
                    {{"test_key_3", "test_value_3"}, {"test_key_4", "test_value_4"}},
                    {{"test_key_1", "test_value_1"},
                     {"test_key_2", "test_value_2"},
                     {"test_key_3", "test_value_3"},
                     {"test_key_4", "test_value_4"}},
                },
                {
                    // Overwrite ContextInfo.
                    "040051b6-440f-4e86-b9ac-70a99169d54c",
                    {{"test_key_2", "test_value_2_overwrite"}},
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"test_key_3", "test_value_3"}},
                    {{"test_key_1", "test_value_1"},
                     {"test_key_2", "test_value_2_overwrite"},
                     {"test_key_3", "test_value_3"}},
                },
                {
                    // Overwrite whole ContextInfo.
                    "2e18f461-7649-44b5-b9d6-1cf5cfd17e08",
                    {{"test_key_1", "test_value_1_overwrite"},
                     {"test_key_2", "test_value_2_overwrite"},
                     {"test_key_3", "test_value_3_overwrite"}},
                    {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"test_key_3", "test_value_3"}},
                    {{"test_key_1", "test_value_1_overwrite"},
                     {"test_key_2", "test_value_2_overwrite"},
                     {"test_key_3", "test_value_3_overwrite"}},
                },
            };

            for (auto& itr_context_info : test_data)
            {
                CAPTURE(itr_context_info.test_id);
                logger.context_info_getter() = itr_context_info.existing_context_info;

                REQUIRE_NOTHROW(logger.add_context_keys(itr_context_info.context_info));
                auto const result_context_info = logger.context_info();
                CAPTURE(ContextInfoEquals::to_string(result_context_info));

                REQUIRE_THAT(result_context_info, ContextInfoEquals(itr_context_info.expected_result));
            }
        }
    }

    SECTION("Remove from ContextInfo")
    {
        logger.add_context_key("test_key_1", "test_value_1");
        logger.add_context_key("test_key_2", "test_value_2");
        REQUIRE_THAT(logger.context_info(),
                     ContextInfoEquals(
                         LoggerMock::ContextInfo({{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}})));

        logger.remove_context_key("test_key_2");
        REQUIRE_THAT(logger.context_info(),
                     ContextInfoEquals(LoggerMock::ContextInfo({{"test_key_1", "test_value_1"}})));

        SECTION("Remove Last")
        {
            logger.remove_context_key("test_key_1");
            REQUIRE_THAT(logger.context_info(), ContextInfoEquals(LoggerMock::ContextInfo({})));
            REQUIRE(logger.context_info().empty());
        }

        SECTION("Remove None Existing")
        {
            logger.add_context_key("test_key_1", "test_value_1");
            REQUIRE_FALSE(logger.context_info().empty());

            auto const test_key = "test_key_none_existing";
            REQUIRE(logger.context_info().find(test_key) == logger.context_info().cend());

            REQUIRE_NOTHROW(logger.remove_context_key(test_key));
            REQUIRE(logger.context_info().find(test_key) == logger.context_info().cend());
        }

        SECTION("Remove From Empty")
        {
            logger.clear_context_info();
            REQUIRE(logger.context_info().empty());

            REQUIRE_NOTHROW(logger.remove_context_key("test_key_from_empty"));
        }
    }

    SECTION("Has Key In ContextInfo")
    {
        struct TestData
        {
            std::string const test_id;
            std::string_view const key;
            LoggerMock::ContextInfo const context_info;
            bool const expected_result;
        };
        std::vector<TestData> test_data{
            {// Empty ContextInfo.
             "4b676284-f384-4914-a35a-f66427e862bd",
             "empty",
             {},
             false},
            {// No such key.
             "4c81fc55-ea57-4ce6-8d6d-f0ec8bffa113",
             "test_no_such_key",
             {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
             false},
            {// Has key.
             "fa37e12c-b7bb-44ac-855f-4553c7a948c4",
             "test_has_key",
             {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"test_has_key", "some_value"}},
             true},
            {// Has empty string key.
             "1931b560-d60e-4c89-8549-cbd70905a017",
             "",
             {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}, {"", "some_value"}},
             true},
            {// Hasn't empty string key.
             "ca953491-19b4-4322-9d05-bc94d03e27c6",
             "",
             {{"test_key_1", "test_value_1"}, {"test_key_2", "test_value_2"}},
             false},
        };

        for (auto& itr_context_info : test_data)
        {
            CAPTURE(itr_context_info.test_id, itr_context_info.key.data(), itr_context_info.expected_result);
            logger.clear_context_info();
            logger.add_context_keys(itr_context_info.context_info);

            bool res;
            REQUIRE_NOTHROW(res = logger.has_context_key(itr_context_info.key));
            REQUIRE(res == itr_context_info.expected_result);
        }
    }

    SECTION("Clear ContextInfo")
    {
        for (int i = 0; i < 5; ++i)
        {
            logger.add_context_key(std::to_string(i), std::to_string(i));
        }
        REQUIRE_FALSE(logger.context_info().empty());

        logger.clear_context_info();
        REQUIRE(logger.context_info().empty());
    }
}
