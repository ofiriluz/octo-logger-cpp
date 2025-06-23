/**
 * @file compat.h
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef COMPAT_HPP
#define COMPAT_HPP
#include <ctime>
#ifdef _WIN32
#include <cerrno>
#endif

namespace octo::logger::compat
{

// Taken from here https://sourceware.org/bugzilla/show_bug.cgi?id=16145
// The reason is that localtime/gmtime internally use __tz_convert, which locks.
// Upon fork, in rare occasions, the child process may inherit the lock,
// causing a deadlock if the child process tries to call localtime.
// To avoid this, we use an internal function that does not lock.
// This is a partial implementation that matches most needs, but does not handle all use cases, 
// like daylight saving time, and leap seconds.
// That is why we use it only in case of utc, with the assumption that the timezone is intentional.
void gmtime_safe_internal(time_t time, long timezone, struct tm *tm_time);
inline struct tm *gmtime_safe(const time_t time, struct tm *tm_time)
{
    gmtime_safe_internal(time, 0, tm_time);
    return tm_time;
}

inline struct tm* localtime(const time_t* timep, struct tm* result, bool safe_utc = false)
{
    if (safe_utc)
    {
        return gmtime_safe(*timep, result);
    }
#ifndef _WIN32
    return localtime_r(timep, result);
#else
    errno = localtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}

} // namespace octo::logger::compat

#endif // COMPAT_HPP