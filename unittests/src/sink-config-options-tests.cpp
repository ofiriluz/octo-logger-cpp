#include <catch2/catch_all.hpp>
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"

using Log = octo::logger::Log;
using Sink = octo::logger::Sink;
using Logger = octo::logger::Logger;
using Channel = octo::logger::Channel;
using ContextInfo = octo::logger::ContextInfo;
using ContextInfoInitializerList = octo::logger::ContextInfo::ContextInfoInitializerList;

class SinkConfigTest: public octo::logger::Sink
{
public:
    using LineFormat = octo::logger::Sink::LineFormat;
    using TimestampFormat = octo::logger::Sink::TimestampFormat;
    using SinkConfig = octo::logger::SinkConfig;

    static LineFormat call_extract_format_with_default(const SinkConfig& config, LineFormat default_format)
    {
        return extract_format_with_default(config, default_format);
    }

    static TimestampFormat call_extract_timestamp_format_with_default(const SinkConfig& config, TimestampFormat default_format)
    {
        return extract_timestamp_format_with_default(config, default_format);
    }

    [[nodiscard]] std::string formatted_context_info(Log const&, Channel const&, ContextInfo const&, ContextInfo const&) const override
    {
        return "";
    }
};

TEST_CASE("SinkConfig option set/get roundtrip", "[sink-config][options]")
{
    using SinkConfig = octo::logger::SinkConfig;
    using SinkType = SinkConfig::SinkType;
    using SinkOption = SinkConfig::SinkOption;
    SinkConfig config("test_sink", SinkType::CONSOLE_JSON_SINK);

    // String option roundtrip
    std::string host = "localhost";
    config.set_option(SinkOption::CONSOLE_JSON_HOST, host);
    REQUIRE(config.option_default(SinkOption::CONSOLE_JSON_HOST, std::string("another_host")) == host);

    // Bool option roundtrip
    config.set_option(SinkOption::LOG_THREAD_ID, true);
    REQUIRE(config.option_default(SinkOption::LOG_THREAD_ID, false) == true);

    // Bool option roundtrip
    config.set_option(SinkOption::LOG_THREAD_ID, false);
    REQUIRE(config.option_default(SinkOption::LOG_THREAD_ID, true) == false);

    // Test extract_format_with_default via SinkConfigTest
    SinkConfig config2("test_sink2", SinkType::CONSOLE_JSON_SINK);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config2, Sink::LineFormat::PLAINTEXT_LONG) == Sink::LineFormat::PLAINTEXT_LONG);
    config2.set_option(SinkOption::LINE_FORMAT, Sink::LineFormat::PLAINTEXT_SHORT);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config2, Sink::LineFormat::PLAINTEXT_LONG) == Sink::LineFormat::PLAINTEXT_SHORT);
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
    config2.set_option(SinkOption::LINE_FORMAT, Sink::LineFormat::JSON);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config2, Sink::LineFormat::PLAINTEXT_LONG) == Sink::LineFormat::JSON);
#endif

    // Test extract_timestamp_format_with_default via SinkConfigTest
    SinkConfig config3("test_sink3", SinkType::CONSOLE_JSON_SINK);
    // First, check the default (should be ISO8601)
    auto val1 = SinkConfigTest::call_extract_timestamp_format_with_default(config3, Sink::TimestampFormat::ISO8601);
    std::cout << "Default timestamp format (should be ISO8601): " << static_cast<int>(val1) << std::endl;
    REQUIRE(val1 == Sink::TimestampFormat::ISO8601);
    // Now set the option and check again (should be UNIX_EPOCH)
    config3.set_option(SinkOption::TIMESTAMP_FORMAT, Sink::TimestampFormat::UNIX_EPOCH);
    auto val2 = SinkConfigTest::call_extract_timestamp_format_with_default(config3, Sink::TimestampFormat::ISO8601);
    std::cout << "Set timestamp format (should be UNIX_EPOCH): " << static_cast<int>(val2) << std::endl;
    REQUIRE(val2 == Sink::TimestampFormat::UNIX_EPOCH);

}

TEST_CASE("SinkConfig extract_format_with_default and extract_timestamp_format_with_default", "[sink-config][options][extract]")
{
    using SinkConfig = octo::logger::SinkConfig;
    using LineFormat = Sink::LineFormat;
    using TimestampFormat = Sink::TimestampFormat;
    SinkConfig config("test_sink", SinkConfig::SinkType::CONSOLE_JSON_SINK);

    // Default value if not set
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config, LineFormat::PLAINTEXT_LONG) == LineFormat::PLAINTEXT_LONG);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config, LineFormat::PLAINTEXT_SHORT) == LineFormat::PLAINTEXT_SHORT);
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config, LineFormat::JSON) == LineFormat::JSON);
#endif
    // Set and get LineFormat
    config.set_option(SinkConfig::SinkOption::LINE_FORMAT, LineFormat::PLAINTEXT_SHORT);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config, LineFormat::PLAINTEXT_LONG) == LineFormat::PLAINTEXT_SHORT);
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
    config.set_option(SinkConfig::SinkOption::LINE_FORMAT, LineFormat::JSON);
    REQUIRE(SinkConfigTest::call_extract_format_with_default(config, LineFormat::PLAINTEXT_LONG) == LineFormat::JSON);
#endif
    
    // Set and get TimestampFormat
    config.set_option(SinkConfig::SinkOption::TIMESTAMP_FORMAT, TimestampFormat::UNIX_EPOCH);
    auto val3 = SinkConfigTest::call_extract_timestamp_format_with_default(config, TimestampFormat::ISO8601);
    std::cout << "Set timestamp format in second test (should be UNIX_EPOCH=" << static_cast<int>(TimestampFormat::UNIX_EPOCH)  << "): " << static_cast<int>(val3) << std::endl;
    REQUIRE(val3 == TimestampFormat::UNIX_EPOCH);
}
