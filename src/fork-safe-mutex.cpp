
/**
 * @file fork-safe-mutex.cpp
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-16
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "octo-logger-cpp/fork-safe-mutex.hpp"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define getpid GetCurrentProcessId
#endif

namespace octo::logger
{

ForkSafeMutex::ForkSafeMutex() : mutex_(std::make_unique<std::mutex>()), mutex_pid_(getpid())
{
}

ForkSafeMutex::~ForkSafeMutex()
{
    fork_reset();
}

void ForkSafeMutex::fork_reset()
{
    if (mutex_pid_ == getpid())
    {
        // Still on parent process, no need to reset
        return;
    }
    if (mutex_->try_lock())
    {
        // Mutex state is good
        mutex_->unlock();
    }
    else
    {
        // Mutex owned by parent only thread, best solution for bad state.
        mutex_.release();
        mutex_ = std::make_unique<std::mutex>();
    }
    mutex_pid_ = getpid();
}

} // namespace octo::logger
