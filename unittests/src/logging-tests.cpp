#include "octo-logger-cpp/logger.hpp"
#include "catch2-matchers.hpp"
#include "octo-logger-cpp/manager.hpp"
#include "dummy-sink.hpp"
#include <catch2/catch_all.hpp>
#include <string>
#include <unordered_map>

namespace
{
using octo::logger::unittests::ContextInfoEquals;
using octo::logger::unittests::DummySink;
using octo::logger::ContextInfo;    
using octo::logger::Logger;

class LoggingTestsFixture
{
  public:
    std::shared_ptr<DummySink> dummy_sink_;
    LoggingTestsFixture(): dummy_sink_(std::make_shared<DummySink>())
    {
        octo::logger::ManagerConfigPtr manager_config = std::make_shared<octo::logger::ManagerConfig>();
        manager_config->add_custom_sink(dummy_sink_);
        octo::logger::Manager::instance().configure(manager_config);
    }
    ~LoggingTestsFixture()
    {
        octo::logger::Manager::reset_manager();
    }
};

} // namespace

TEST_CASE_METHOD(LoggingTestsFixture, "Logger Context Tests", "[logger]")
{
    Logger logger("logging-tests");

    SECTION("Init")
    {
        REQUIRE(logger.context_info().empty());
    }

    SECTION("Update Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        logger.add_context_keys(ci);
        logger.info("Test log");
        REQUIRE_FALSE(logger.context_info().empty());
        REQUIRE(logger.context_info().contains("key1"));
        REQUIRE(logger.context_info().contains("key2"));
        REQUIRE(dummy_sink_->last_log().context_info == ci);
    }

    SECTION("Erase Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        logger.add_context_keys(ci);
        logger.remove_context_key("key1");
        logger.info("Test log");
        REQUIRE_FALSE(logger.context_info().contains("key1"));
        REQUIRE(logger.context_info().contains("key2"));
        REQUIRE(dummy_sink_->last_log().context_info.contains("key2"));
        REQUIRE_FALSE(dummy_sink_->last_log().context_info.contains("key1"));
    }

    SECTION("Clear Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        logger.add_context_keys(ci);
        logger.clear_context_info();
        logger.info("Test log");
        REQUIRE(logger.context_info().empty());
        REQUIRE(dummy_sink_->last_log().context_info.empty());
    }   
}

TEST_CASE_METHOD(LoggingTestsFixture, "Logger Manager Global Context Info Tests", "[logger]")
{
    auto& manager = octo::logger::Manager::instance();

    SECTION("Init")
    {
        REQUIRE(manager.global_context_info().empty());
    }

    SECTION("Update Global Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        manager.replace_global_context_info(ci);
        Logger logger("logging-tests");
        logger.info("Test log");
        REQUIRE_FALSE(manager.global_context_info().empty());
        REQUIRE(manager.global_context_info().contains("key1"));
        REQUIRE(manager.global_context_info().contains("key2"));
        REQUIRE(dummy_sink_->last_log().global_context_info == ci);
        REQUIRE(dummy_sink_->last_log().global_context_info_addr == &manager.global_context_info());
    }

    SECTION("With Logger Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        manager.replace_global_context_info(ci);
        Logger logger("logging-tests");
        logger.add_context_keys({{"key3", "value3"}});
        logger.info("Test log");
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key1"));
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key2"));
        REQUIRE(dummy_sink_->last_log().context_info.contains("key3"));
        REQUIRE(dummy_sink_->last_log().context_info_addr == &logger.context_info());
        REQUIRE(dummy_sink_->last_log().global_context_info_addr == &manager.global_context_info());
    }

    SECTION("With Log Context Info")
    {
        ContextInfo ci{{"key1", "value1"}, {"key2", "value2"}};
        manager.replace_global_context_info(ci);
        Logger logger("logging-tests");
        logger.add_context_keys({{"key3", "value3"}, {"key4", "value4"}});
        logger.info("Test log", {{"key5", "value5"}});
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key1"));
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key2"));
        REQUIRE(dummy_sink_->last_log().global_context_info_addr == &manager.global_context_info());
        REQUIRE(dummy_sink_->last_log().context_info.contains("key3"));
        REQUIRE(dummy_sink_->last_log().context_info.contains("key4"));
        REQUIRE(dummy_sink_->last_log().context_info_addr == &logger.context_info());
        REQUIRE(dummy_sink_->last_log().log_context_info.contains("key5"));
    }

    SECTION("With New Context Info")
    {
        auto old_ci = manager.global_context_info();
        ContextInfo ci{{"key5", "value5"}};
        manager.replace_global_context_info(ci);
        manager.update_global_context_info({{"key6", "value6"}});
        Logger logger("logging-tests");
        logger.add_context_keys({{"key3", "value3"}});
        logger.info("Test log", {{"key4", "value4"}});
        REQUIRE_FALSE(dummy_sink_->last_log().global_context_info.contains("key1"));
        REQUIRE_FALSE(dummy_sink_->last_log().global_context_info.contains("key2"));
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key5"));
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key6"));
        REQUIRE(dummy_sink_->last_log().global_context_info_addr == &manager.global_context_info());
        REQUIRE(dummy_sink_->last_log().context_info.contains("key3"));
        REQUIRE(dummy_sink_->last_log().log_context_info.contains("key4"));
        manager.replace_global_context_info(old_ci);
    }

    SECTION("Update Manager Context ")
    {
        ContextInfo ci{manager.global_context_info()};
        ci.update("key5", "value5");
        manager.replace_global_context_info(ci);
        Logger logger("logging-tests");
        logger.add_context_keys({{"key3", "value3"}});
        logger.info("Test log", {{"key4", "value4"}});
        REQUIRE_FALSE(dummy_sink_->last_log().global_context_info.contains("key1"));
        REQUIRE_FALSE(dummy_sink_->last_log().global_context_info.contains("key2"));
        REQUIRE(dummy_sink_->last_log().context_info.contains("key3"));
        REQUIRE(dummy_sink_->last_log().log_context_info.contains("key4"));
        REQUIRE(dummy_sink_->last_log().global_context_info.contains("key5"));
    }
}
