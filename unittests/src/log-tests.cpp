#include "octo-logger-cpp/log.hpp"
#include "catch2-matchers.hpp"
#include "octo-logger-cpp/manager.hpp"
#include "log-mock.hpp"
#include "logger-mock.hpp"
#include <catch2/catch_all.hpp>
#include <string>
#include <unordered_map>

namespace
{
using octo::logger::unittests::LogLevelEquals;
using octo::logger::unittests::LogMock;
using LogLevel = octo::logger::unittests::LogMock::LogLevel;
using LoggerMock = octo::logger::Logger::LoggerMock;

class LogTestsFixture
{
  public:
    LoggerMock logger_trace_;
    LoggerMock logger_debug_;
    LoggerMock logger_info_;
    LoggerMock logger_notice_;
    LoggerMock logger_warning_;
    LoggerMock logger_error_;
    LoggerMock logger_quiet_;

  public:
    LogTestsFixture()
        : logger_trace_("logger_trace_channel"),
          logger_debug_("logger_debug_channel"),
          logger_info_("logger_info_channel"),
          logger_notice_("logger_notice_channel"),
          logger_warning_("logger_warning_channel"),
          logger_error_("logger_error_channel"),
          logger_quiet_("logger_quiet_channel")
    {
        logger_trace_.editable_logger_channel().set_log_level(LogLevel::TRACE);
        logger_debug_.editable_logger_channel().set_log_level(LogLevel::DEBUG);
        logger_info_.editable_logger_channel().set_log_level(LogLevel::INFO);
        logger_notice_.editable_logger_channel().set_log_level(LogLevel::NOTICE);
        logger_warning_.editable_logger_channel().set_log_level(LogLevel::WARNING);
        logger_error_.editable_logger_channel().set_log_level(LogLevel::ERROR);
        logger_quiet_.editable_logger_channel().set_log_level(LogLevel::QUIET);
    }
    ~LogTestsFixture()
    {
        octo::logger::Manager::reset_manager();
    }
};

} // namespace

TEST_CASE_METHOD(LogTestsFixture, "Log Initialization Tests", "[log]")
{
    SECTION("Initialize LogLevel")
    {
        LogMock const log_trace(LogLevel::TRACE, "log_trace", logger_trace_);
        REQUIRE_THAT(log_trace.log_level(), LogLevelEquals(LogLevel::TRACE));

        LogMock const log_debug(LogLevel::DEBUG, "log_debug", logger_debug_);
        REQUIRE_THAT(log_debug.log_level(), LogLevelEquals(LogLevel::DEBUG));

        LogMock const log_info(LogLevel::INFO, "log_info", logger_info_);
        REQUIRE_THAT(log_info.log_level(), LogLevelEquals(LogLevel::INFO));

        LogMock const log_notice(LogLevel::NOTICE, "log_notice", logger_notice_);
        REQUIRE_THAT(log_notice.log_level(), LogLevelEquals(LogLevel::NOTICE));

        LogMock const log_warning(LogLevel::WARNING, "log_warning", logger_warning_);
        REQUIRE_THAT(log_warning.log_level(), LogLevelEquals(LogLevel::WARNING));

        LogMock const log_error(LogLevel::ERROR, "log_error", logger_error_);
        REQUIRE_THAT(log_error.log_level(), LogLevelEquals(LogLevel::ERROR));

        LogMock const log_quiet(LogLevel::QUIET, "log_quiet", logger_quiet_);
        REQUIRE_THAT(log_quiet.log_level(), LogLevelEquals(LogLevel::QUIET));
    }
}
