/**
 * @file fork-safe-mutex.h
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
private:
    std::unique_ptr<std::mutex> mutex_;
    pid_t mutex_pid_;

public:
    ForkSafeMutex();
    void lock();
    [[nodiscard]] bool try_lock(); 
    void unlock();
    /**
     * @brief Resets the mutex after a fork.
     * Should only be called if currently there are no additional threads using the mutex.
     */
    void fork_reset();
private:
    ForkSafeMutex(ForkSafeMutex&&)            = delete;
    ForkSafeMutex(ForkSafeMutex&&)            = delete;
    ForkSafeMutex& operator=(ForkSafeMutex&&) = delete;
    ForkSafeMutex& operator=(ForkSafeMutex&&) = delete;

};

} // namespace octo::logger

#endif