/**
 * @file fork-safe-mutex.hpp
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-14
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FORK_SAFE_MUTEX_HPP_
#define FORK_SAFE_MUTEX_HPP_

#include <memory>
#include <mutex>

namespace octo::logger
{

class ForkSafeMutex
{
  public:
    using MutexType = std::mutex;

  private:
    std::unique_ptr<MutexType> mutex_;
#ifdef _WIN32
    typedef std::uint32_t pid_t;
#endif
    pid_t mutex_pid_;

  public:
    explicit ForkSafeMutex();
    ~ForkSafeMutex();

    // Non-copyable and non-movable
    ForkSafeMutex(ForkSafeMutex&&) = delete;
    ForkSafeMutex& operator=(ForkSafeMutex&&) = delete;
    ForkSafeMutex(const ForkSafeMutex&) = delete;
    ForkSafeMutex& operator=(const ForkSafeMutex&) = delete;

    /// @brief Implicit conversion to MutexType
    operator MutexType&()
    {
        return *mutex_;
    }

    /**
     * @brief Resets the mutex after a fork.
     * Should only be called if currently there are no additional threads using the mutex.
     * If the mutex is locked by another thread, then it will be purposefully leaked and replaced by a newly allocated
     * mutex. We leak it since destroying a locked mutex is undefined behavior.
     */
    void fork_reset();
};

} // namespace octo::logger

#endif
