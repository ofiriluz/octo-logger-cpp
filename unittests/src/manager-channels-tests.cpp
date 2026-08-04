#include "octo-logger-cpp/manager.hpp"
#include "octo-logger-cpp/channel.hpp"
#include <catch2/catch_all.hpp>
#include <atomic>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

class ManagerChannelsFixture
{
  public:
    ManagerChannelsFixture() = default;
    ~ManagerChannelsFixture()
    {
        octo::logger::Manager::reset_manager();
    }
};

} // namespace

// ----- Deterministic string_view key-correctness tests -----
// These expose the name.data() bug in create_channel without any threading.

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: non-NUL-terminated string_view produces correct channel name",
                 "[manager][channels][string_view]")
{
    auto& mgr = octo::logger::Manager::instance();

    // Build a view that does NOT end at a NUL boundary.
    // name.data() would return the full buffer "alpha_beta", but the view's
    // length is 5 ("alpha"), so the channel_name_ must be "alpha" after the fix.
    std::string buf{"alpha_beta"};
    std::string_view short_view{buf.data(), 5}; // "alpha"

    mgr.create_channel(short_view);

    // Pre-fix: key is "alpha_beta" (name.data()), so has_channel("alpha") is false.
    // Post-fix: key is "alpha", so this must be true.
    REQUIRE(mgr.has_channel("alpha"));
    REQUIRE_FALSE(mgr.has_channel("alpha_beta"));

    // The channel's own name must also agree.
    REQUIRE(mgr.channel("alpha").channel_name() == "alpha");
}

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: two different-length views into the same buffer yield distinct channels",
                 "[manager][channels][string_view]")
{
    auto& mgr = octo::logger::Manager::instance();

    std::string buf{"foo_bar"};
    std::string_view view_foo{buf.data(), 3}; // "foo"
    std::string_view view_foo_bar{buf.data(), 7}; // "foo_bar"

    mgr.create_channel(view_foo);
    mgr.create_channel(view_foo_bar);

    // Pre-fix: both calls use name.data() == "foo_bar" as the key, so only one
    // channel is created and has_channel("foo") returns false.
    // Post-fix: two distinct channels exist.
    REQUIRE(mgr.has_channel("foo"));
    REQUIRE(mgr.has_channel("foo_bar"));

    // The two ChannelPtrs must be different objects.
    REQUIRE(&mgr.channel("foo") != &mgr.channel("foo_bar"));
}

// ----- Concurrency tests -----
// These expose the unsynchronized access to channels_.
// Under TSan they produce DATA RACE reports; without TSan they are
// probabilistic but reliably crash in debug builds with enough threads.

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: concurrent creation of many distinct channels does not corrupt the map",
                 "[manager][channels][concurrency]")
{
    auto& mgr = octo::logger::Manager::instance();

    constexpr int NUM_THREADS = 16;
    constexpr int CHANNELS_PER_THREAD = 50;

    std::atomic<bool> go{false};
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire))
            {
            }
            for (int c = 0; c < CHANNELS_PER_THREAD; ++c)
            {
                std::string name = "ch_t" + std::to_string(t) + "_c" + std::to_string(c);
                mgr.create_channel(name);
            }
        });
    }

    go.store(true, std::memory_order_release);
    for (auto& th : threads)
    {
        th.join();
    }

    // Every channel must be findable and have the correct name after all threads finish.
    for (int t = 0; t < NUM_THREADS; ++t)
    {
        for (int c = 0; c < CHANNELS_PER_THREAD; ++c)
        {
            std::string name = "ch_t" + std::to_string(t) + "_c" + std::to_string(c);
            REQUIRE(mgr.has_channel(name));
            REQUIRE(mgr.channel(name).channel_name() == name);
        }
    }
}

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: concurrent creation of the same name returns the same Channel object",
                 "[manager][channels][concurrency]")
{
    auto& mgr = octo::logger::Manager::instance();

    constexpr int NUM_THREADS = 32;
    const std::string channel_name = "shared_channel";

    std::atomic<bool> go{false};
    // Collect the Channel* each thread sees (via the returned ChannelView).
    std::vector<const octo::logger::Channel*> results(NUM_THREADS, nullptr);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire))
            {
            }
            auto view = mgr.create_channel(channel_name);
            results[t] = &view.channel();
        });
    }

    go.store(true, std::memory_order_release);
    for (auto& th : threads)
    {
        th.join();
    }

    // All threads must have received a pointer to the same Channel instance.
    const octo::logger::Channel* expected = results[0];
    REQUIRE(expected != nullptr);
    for (int t = 1; t < NUM_THREADS; ++t)
    {
        REQUIRE(results[t] == expected);
    }
}

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: racing with has_channel, mute_channel, and set_log_level does not crash",
                 "[manager][channels][concurrency]")
{
    auto& mgr = octo::logger::Manager::instance();

    // Pre-populate some channels so readers have something to find.
    for (int i = 0; i < 20; ++i)
    {
        mgr.create_channel("pre_" + std::to_string(i));
    }

    constexpr int ITERATIONS = 200;
    std::atomic<bool> go{false};

    auto writer = std::thread([&]() {
        while (!go.load(std::memory_order_acquire))
        {
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            mgr.create_channel("new_" + std::to_string(i));
        }
    });

    auto reader_has = std::thread([&]() {
        while (!go.load(std::memory_order_acquire))
        {
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            (void)mgr.has_channel("pre_" + std::to_string(i % 20));
        }
    });

    auto reader_mute = std::thread([&]() {
        while (!go.load(std::memory_order_acquire))
        {
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            mgr.mute_channel("pre_" + std::to_string(i % 20));
        }
    });

    auto reader_level = std::thread([&]() {
        while (!go.load(std::memory_order_acquire))
        {
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            mgr.set_log_level(octo::logger::Log::LogLevel::DEBUG);
        }
    });

    go.store(true, std::memory_order_release);
    writer.join();
    reader_has.join();
    reader_mute.join();
    reader_level.join();

    // If we got here without a crash or TSan report, the test passes.
    REQUIRE(true);
}

// ----- Fork regression guard -----
// Verifies that channels_mutex_.fork_reset() is called in child_on_fork(),
// so a child that acquires channels_mutex_ after fork does not deadlock.
// This is NOT a pre-fix failure test; it guards the new mutex against fork.

TEST_CASE_METHOD(ManagerChannelsFixture,
                 "create_channel: child process can create channels after child_on_fork without deadlock",
                 "[manager][channels][fork]")
{
    auto& mgr = octo::logger::Manager::instance();

    // Acquire the mutex in the parent to simulate a worst-case fork: the mutex
    // is held at fork time. We do this by creating a channel just before fork,
    // then releasing (the lock_guard exits).
    mgr.create_channel("parent_channel");

    pid_t const pid = fork();
    REQUIRE(pid >= 0);

    if (pid == 0)
    {
        // Child: reset the manager's fork-unsafe state, then exercise create_channel.
        octo::logger::Manager::instance().child_on_fork();
        octo::logger::Manager::instance().create_channel("child_channel");
        // Must not deadlock. If we reach _exit(0), success.
        _exit(0);
    }

    // Parent: wait for child with a reasonable timeout.
    int status = 0;
    // Wait up to 5 seconds for the child.
    for (int i = 0; i < 50; ++i)
    {
        int ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid)
        {
            break;
        }
        if (ret < 0)
        {
            // waitpid error
            kill(pid, SIGKILL);
            FAIL("waitpid failed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // If child is still alive after 5 s, it deadlocked.
    int ret = waitpid(pid, &status, WNOHANG);
    if (ret == 0)
    {
        kill(pid, SIGKILL);
        FAIL("Child deadlocked after child_on_fork — channels_mutex_ was not reset");
    }

    REQUIRE(WIFEXITED(status));
    REQUIRE(WEXITSTATUS(status) == 0);
}
