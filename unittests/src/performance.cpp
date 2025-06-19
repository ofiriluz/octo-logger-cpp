#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/manager.hpp"
#include "logger-mock.hpp"
#include <catch2/catch_all.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <signal.h>

class LoggerPerformanceFixture {
public:
    LoggerPerformanceFixture() = default;
    ~LoggerPerformanceFixture() {
        octo::logger::Manager::reset_manager();
    }
};

TEST_CASE_METHOD(LoggerPerformanceFixture, "Logger fork performance test: child does not hang after first log", "[logger][fork][performance]")
{
    constexpr int NUM_FORKS = 1000;
    octo::logger::unittests::LoggerMock logger("fork_perf_logger");
    std::vector<pid_t> children;

    // Start a background thread in the parent that logs constantly
    std::atomic<bool> running{true};
    std::thread bg_logger([&logger, &running]() {
        int count = 0;
        while (running) {
            logger.info().formatted(FMT_STRING("Parent background log {}"), count++);
        }
    });

    for (int i = 0; i < NUM_FORKS; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            octo::logger::Manager::instance().child_on_fork();
            logger.info().formatted(FMT_STRING("Child {} log"), pid);
            _exit(0);
        } else if (pid > 0) {
            // Parent process: log to ensure logging threads are alive
            logger.info().formatted(FMT_STRING("Parent after fork {}"), i);
            children.push_back(pid);
        } else {
            FAIL("fork() failed");
        }
    }

    // Wait 5 seconds before reaping children
    std::this_thread::sleep_for(std::chrono::seconds(5));
    int failed_waits = 0;
    for (pid_t pid : children) {
        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);
        if (ret == 0) {
            // Child still running after 2 seconds
            ++failed_waits;
        } else if (ret == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Success
        } else {
            ++failed_waits;
        }
    }
    if (failed_waits > 0) {
        std::cerr << "Failed waits: " << failed_waits << std::endl;
        // Kill all remaining children just in case
        for (pid_t pid : children) {
            kill(pid, 9);
        }
        FAIL(std::to_string(failed_waits) + " children did not exit cleanly");
    }

    // Stop background logger
    running = false;
    bg_logger.join();
}
