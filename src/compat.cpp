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
#include <cstdint>
#include <iostream>
#include "octo-logger-cpp/compat.hpp"
namespace octo::logger::compat
{

namespace
{

static struct tm* localtime_internal(const time_t* timep, struct tm* result)
{
#ifndef _WIN32
    return localtime_r(timep, result);
#else
    errno = localtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}


static struct tm* gmtime_internal(const time_t* timep, struct tm* result)
{
#ifndef _WIN32
    return gmtime_r(timep, result);
#else
    errno = gmtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}

static bool calculate_is_utc() 
{
    time_t now = time(nullptr);
    struct tm gm_timeinfo;
    struct tm local_timeinfo;
    struct tm *local = localtime_internal(&now, &local_timeinfo);
    struct tm *gmt = gmtime_internal(&now, &gm_timeinfo);
    std::cerr << mktime(local) - mktime(gmt) << " seconds difference." << std::endl;
    return static_cast<int>(mktime(local) - mktime(gmt)) == 0;
}

// See https://sourceware.org/bugzilla/show_bug.cgi?id=16145
static void gmtime_safe_internal(time_t time, long timezone, struct tm *tm_time) 
{
    const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    std::uint32_t n32_Pass4year;
    std::uint32_t n32_hpery;

    time=time + (timezone * 60 * 60);

    if(time < 0)
    {
        time = 0;
    }
    tm_time->tm_sec=(int)(time % 60);
    time /= 60;
    tm_time->tm_min=(int)(time % 60);
    time /= 60;
    n32_Pass4year=((unsigned int)time / (1461L * 24L));
    tm_time->tm_year=(n32_Pass4year << 2)+70;
    time %= 1461L * 24L;
    for (;;)
    {
        n32_hpery = 365 * 24;
        if ((tm_time->tm_year & 3) == 0)
        {
            n32_hpery += 24;
        }
        if (time < n32_hpery)
        {
            break;
        }
        tm_time->tm_year++;
        time -= n32_hpery;
    }
    tm_time->tm_hour=(int)(time % 24);
    time /= 24;
    time++;
    if ((tm_time->tm_year & 3) == 0)
    {
        if (time > 60)
        {
            time--;
        }
        else
        {
            if (time == 60)
            {
                tm_time->tm_mon = 1;
                tm_time->tm_mday = 29;
                return;
            }
        }
    }
    for (tm_time->tm_mon = 0; Days[tm_time->tm_mon] < time;tm_time->tm_mon++)
    {
        time -= Days[tm_time->tm_mon];
    }

    tm_time->tm_mday = (int)(time);
    return;
}

} // namespace


// Taken from here https://sourceware.org/bugzilla/show_bug.cgi?id=16145
struct tm *gmtime_safe(time_t time, struct tm *tm_time)
{
    // The reason is that localtime/gmtime internally use __tz_convert, which locks.
    // Upon fork, in rare occasions, the child process may inherit the lock,
    // causing a deadlock if the child process tries to call localtime.
    // To avoid this, we use an internal function that does not lock.
    // You can find source here https://sourceware.org/bugzilla/show_bug.cgi?id=16145
    // This is a partial implementation that matches most needs, but does not handle all use cases, 
    // like daylight saving time, and leap seconds.
    // That is why we use it only in case of utc, with the assumption that the timezone is intentional.
    gmtime_safe_internal(time, 0, tm_time);
    return tm_time;
}

struct tm* localtime(const time_t* timep, struct tm* result)
{
    const static long is_utc = calculate_is_utc();
    // gmtime_safe is a partial implementation that matches most needs, but does not handle all use cases, 
    // like daylight saving time, and leap seconds.
    // That is why we use it only in case of utc, with the assumption that the timezone is intentional.
    return is_utc ? gmtime_safe(*timep, result):
            localtime_internal(timep, result);
}

} // namespace octo::logger::compat
