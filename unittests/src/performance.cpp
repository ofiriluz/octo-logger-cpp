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
    auto config = std::make_shared<octo::logger::ManagerConfig>();
    config->set_option(octo::logger::ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL,
                       octo::logger::Log::LogLevel::INFO);
    octo::logger::SinkConfig console_sink("Console", octo::logger::SinkConfig::SinkType::CONSOLE_JSON_SINK);
    console_sink.set_option(octo::logger::SinkConfig::SinkOption::CONSOLE_JSON_HOST, "localhost");
    console_sink.set_option(octo::logger::SinkConfig::SinkOption::LOG_THREAD_ID, true);
    config->add_sink(console_sink);
    octo::logger::Manager::instance().configure(config);

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
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < NUM_FORKS; ++i) {
        octo::logger::Manager::instance().execute_pre_fork();
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            octo::logger::Manager::instance().execute_post_fork(true);
            std::cerr << "Child process started with pid: " << getpid() << std::endl;
            logger.info().formatted(FMT_STRING("Child {} log"), getpid());
            _exit(0);
        } else if (pid > 0) {
            // Parent process: log to ensure logging threads are alive
            octo::logger::Manager::instance().execute_post_fork(false);
            logger.info().formatted(FMT_STRING("Parent after fork {}"), pid);
            children.push_back(pid);
        } else {
            octo::logger::Manager::instance().execute_post_fork(true);
            FAIL("fork() failed");
        }
    }
    running = false;
    bg_logger.join();
    // Wait 5 seconds before reaping children
    std::this_thread::sleep_for(std::chrono::seconds(5));
    int failed_waits = 0;
    std::vector<pid_t> failed_pids;
    for (pid_t pid : children) {
        int status = 0;
        int ret = waitpid(pid, &status, WNOHANG);
        if (ret == 0) {
            // Child still running after 5 seconds
            ++failed_waits;
            failed_pids.push_back(pid);
            std::cerr << "Failed waits: " << failed_waits << " pid: " << pid << std::endl;
        } else if (ret == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Success
        } else {
            ++failed_waits;
            failed_pids.push_back(pid);
            std::cerr << "Failed waits: " << failed_waits << " pid: " << pid << std::endl;
        }
    }

    if (failed_waits > 0) {
        std::cerr << "Failed waits: " << failed_waits << std::endl;
        // Kill only failed children just in case
        for (pid_t pid : failed_pids) {
            std::cerr << "Killing child pid: " << pid << std::endl;
            kill(pid, 9);
        }
        FAIL(std::to_string(failed_waits) + " children did not exit cleanly");
    }

    // Stop background logger

}
