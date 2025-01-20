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

#include <mutex>
#include <memory>

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
    ForkSafeMutex();
    virtual ~ForkSafeMutex();
    ForkSafeMutex(ForkSafeMutex&&) = delete;
    ForkSafeMutex& operator=(ForkSafeMutex&&) = delete;
    ForkSafeMutex(const ForkSafeMutex&) = delete;
    ForkSafeMutex& operator=(const ForkSafeMutex&) = delete;

    inline MutexType& operator*()
    {
        return *mutex_;
    }

    inline MutexType& get()
    {
        return *mutex_;
    }

    /**
     * @brief Resets the mutex after a fork.
     * Should only be called if currently there are no additional threads using the mutex.
     */
    void fork_reset();
};

} // namespace octo::logger

#endif
