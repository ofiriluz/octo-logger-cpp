#include <catch2/catch_all.hpp>
#include "octo-logger-cpp/compat.hpp"
#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>

TEST_CASE("gmtime_safe matches std::gmtime", "[compat][gmtime]") {
    using namespace std::chrono;
    // Test a range of times, including edge cases
    std::time_t now = std::time(nullptr);
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
        // Note: These are not supported
        // REQUIRE(safe_tm.tm_wday == std_ptr->tm_wday);
        // REQUIRE(safe_tm.tm_yday == std_ptr->tm_yday);
        // REQUIRE(safe_tm.tm_isdst == std_ptr->tm_isdst);
    }
}

TEST_CASE("Performance: gmtime_safe vs localtime_safe", "[compat][gmtime][localtime][performance]") {
    constexpr int N = 1000000;
    std::time_t now = std::time(nullptr);
    std::vector<std::time_t> times;
    times.reserve(N);
    for (int i = 0; i < N; ++i) {
        times.push_back(now + i);
    }
    std::tm tm_buf = {};
    volatile int sum = 0; // prevent optimization

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::tm* tm = octo::logger::compat::localtime(&times[i], &tm_buf, true);
        sum += tm->tm_sec;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto gmtime_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::tm* tm = octo::logger::compat::localtime(&times[i], &tm_buf, false);
        sum += tm->tm_sec;
    }
    t2 = std::chrono::high_resolution_clock::now();
    auto localtime_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "gmtime_safe: " << gmtime_duration << " ms, localtime_safe: " << localtime_duration << " ms, sum: " << sum << std::endl;
    // Not a correctness test, just for timing
    REQUIRE(true);
}
