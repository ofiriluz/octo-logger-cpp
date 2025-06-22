/**
 * @file compat.cpp
 * @author Arad Yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "octo-logger-cpp/compat.hpp"

namespace octo::logger::compat
{

namespace
{

struct tm* localtime_internal(const time_t* timep, struct tm* result)
{
#ifndef _WIN32
    return localtime_r(timep, result);
#else
    errno = localtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}


struct tm* gmtime_internal(const time_t* timep, struct tm* result)
{
#ifndef _WIN32
    return gmtime_r(timep, result);
#else
    errno = gmtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}

static bool check_is_utc() 
{
    time_t now = time(nullptr);
    struct tm gm_timeinfo;
    struct tm local_timeinfo;
    struct tm *local = localtime_internal(&now, &local_timeinfo);
    struct tm *gmt = gmtime_internal(&now, &gm_timeinfo);
    return static_cast<int>(mktime(local) - mktime(gmt)) != 0;
}

} // namespace

struct tm* localtime(const time_t* timep, struct tm* result)
{
    const static bool is_utc = check_is_utc();
    // The reason is that localtime internally use __tz_convert, which locks.
    // Upon fork, in rare occasions, the child process may inherit the lock,
    // causing a deadlock if the child process tries to call localtime.
    // To avoid this, we check if the system is using UTC or not.
    // If it is, we use gmtime instead of localtime.
    // NOTE: This is not a perfect solution, as gmtime also locks, but for less time.
    // Check the web, this is a known issue with localtime and fork.
    return is_utc ? gmtime_internal(timep, result) : localtime_internal(timep, result);
}

} // namespace octo::logger::compat
