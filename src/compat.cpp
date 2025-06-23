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

// See https://sourceware.org/bugzilla/show_bug.cgi?id=16145
void gmtime_safe_internal(time_t time, long timezone, struct tm *tm_time) 
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

} // namespace octo::logger::compat
