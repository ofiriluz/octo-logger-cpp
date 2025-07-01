#include <catch2/catch_all.hpp>
#include "octo-logger-cpp/compat.hpp"
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <thread>
#include <iostream>

TEST_CASE("gmtime_safe matches std::gmtime", "[compat][gmtime]") {
    using namespace std::chrono;
    // Test a range of times, including edge cases
    std::time_t const now = std::time(nullptr);
    std::time_t times[] = {
        0, // Epoch
        now,
        now - 86400, // 1 day ago
        now + 86400, // 1 day ahead
        2147483647, // Year 2038 problem boundary (on 32-bit)
    };
    for (std::time_t t : times) {
        std::tm safe_tm = {};
        std::tm* safe_ptr = octo::logger::compat::gmtime_safe(t, &safe_tm);
        std::tm* std_ptr = std::gmtime(&t);
        REQUIRE(safe_ptr != nullptr);
        REQUIRE(std_ptr != nullptr);
        // Compare all fields
        REQUIRE(safe_tm.tm_sec == std_ptr->tm_sec);
        REQUIRE(safe_tm.tm_min == std_ptr->tm_min);
        REQUIRE(safe_tm.tm_hour == std_ptr->tm_hour);
        REQUIRE(safe_tm.tm_mday == std_ptr->tm_mday);
        REQUIRE(safe_tm.tm_mon == std_ptr->tm_mon);
        REQUIRE(safe_tm.tm_year == std_ptr->tm_year);
        // Note: These are not supported in the current implementation of safe gmtime
        // REQUIRE(safe_tm.tm_wday == std_ptr->tm_wday);
        // REQUIRE(safe_tm.tm_yday == std_ptr->tm_yday);
        // REQUIRE(safe_tm.tm_isdst == std_ptr->tm_isdst);
    }
}

// Helper function to format time as ISO 8601 with milliseconds and timezone offset
// Should be same as how we serialize the timestamp for our Cloudwatch Sink and for the Console JSON Sink
std::string format_time(std::tm const* timeinfo) {
    std::stringstream ss;
    // Put datetime with milliseconds: YYYY-MM-DDTHH:MM:SS.mmm
    ss << std::put_time(timeinfo, "%FT%T");
    std::chrono::milliseconds const ms{123}; // Arbitrary milliseconds
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    // Put timezone as offset from UTC: ±HHMM
    ss << std::put_time(timeinfo, "%z");
    return ss.str();
}

// Fixture to set the timezone to UTC for testing
// Resets the original timezone after the test
struct SetUTCTimezoneFixture {
    std::string old_tz;

    SetUTCTimezoneFixture() {
        const char* tz = getenv("TZ");
        old_tz = tz ? tz : "";

        std::cout << "Setting timezone to UTC for tests" << std::endl;
        setenv("TZ", "UTC", 1);
        tzset();
    }

    ~SetUTCTimezoneFixture() {
        if (!old_tz.empty()) {
            std::cout << "Resetting timezone to " << old_tz << std::endl;
            setenv("TZ", old_tz.c_str(), 1);
        } else {
            std::cout << "Unsetting TZ env var" << std::endl;
            unsetenv("TZ");
        }
        tzset();
    }
};

TEST_CASE_METHOD(SetUTCTimezoneFixture, "Stringify compat::localtime with std::put_time with/without safe_utc enabled should match", "[compat][gmtime][put_time]") {
    using namespace std::chrono;
    // Test a range of times, including edge cases
    std::time_t const now = std::time(nullptr);
    std::time_t times[] = {
        0, // Epoch
        now,
        now - 86400, // 1 day ago
        now + 86400, // 1 day ahead
        2147483647, // Year 2038 problem boundary (on 32-bit)
    };
    for (std::time_t t : times) {
        std::tm safe_localtime_tm = {};
        std::tm const* safe_localtime_ptr = octo::logger::compat::localtime(&t, &safe_localtime_tm, true);
        std::tm unsafe_localtime_tm = {};
        std::tm const* unsafe_localtime_ptr = octo::logger::compat::localtime(&t, &unsafe_localtime_tm, false);
        REQUIRE(safe_localtime_ptr != nullptr);
        REQUIRE(unsafe_localtime_ptr != nullptr);

        // Compare stringified times
        std::string const safe_localtime_str = format_time(safe_localtime_ptr);
        std::string const unsafe_localtime_str = format_time(unsafe_localtime_ptr);
        std::cout << "Safe time: " << safe_localtime_str << ", Std time: " << unsafe_localtime_str << std::endl;
        REQUIRE(safe_localtime_str == unsafe_localtime_str);
    }
}

TEST_CASE("Performance: gmtime_safe vs localtime_safe", "[compat][gmtime][localtime][performance]") {
    constexpr int N = 1'000'000;
    std::time_t const now = std::time(nullptr);
    std::vector<std::time_t> times;
    times.reserve(N);
    for (int i = 0; i < N; ++i) {
        times.push_back(now + rand() % now);
    }
    std::tm tm_buf = {};

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::tm* tm = octo::logger::compat::localtime(&times[i], &tm_buf, true);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto const gmtime_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::tm* tm = octo::logger::compat::localtime(&times[i], &tm_buf, false);
    }
    t2 = std::chrono::high_resolution_clock::now();
    auto const localtime_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "gmtime_safe: " << gmtime_duration << " ms, localtime_safe: " << localtime_duration << " ms" << std::endl;
    // Not a correctness test, just for timing
    REQUIRE(true);
}

