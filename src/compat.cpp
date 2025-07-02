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
    std::uint32_t n32_Pass4year = 0;
    std::uint32_t n32_hpery = 0;

    time=time + (timezone * 60 * 60);

    if(time < 0)
    {
        time = 0;
    }
    tm_time->tm_sec=(int)(time % 60);
    time /= 60;
    tm_time->tm_min=(int)(time % 60);
    time /= 60;
    // Find number of 4-year periods (1461 days) passed since the epoch
    n32_Pass4year=((unsigned int)time / (1461L * 24L));
    // Multiply number of 4-year periods by 4 in order to get the number of years passed since the epoch, and add 70 because the epoch year is 1970
    tm_time->tm_year=(n32_Pass4year << 2)+70;
    time %= 1461L * 24L;
    for (;;)
    {
        n32_hpery = 365 * 24;
        // Check for Leap year
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
    // At this point `time` represents the number of days passed since the epoch
    time /= 24;
    // Assumed to be due to days starting from 1, so we add 1 to the day count
    time++;
    if ((tm_time->tm_year & 3) == 0)
    {
        if (time > 60)
        {
            // Decrement the day by 1 to account for the extra day in February, so that the day in the month is calculated correctly below
            time--;
        }
        else
        {
            // Account for the edge case of February 29th on a leap year
            if (time == 60)
            {
                tm_time->tm_mon = 1;
                tm_time->tm_mday = 29;
                return;
            }
        }
    }
    // Find the month and day in the month
    for (tm_time->tm_mon = 0; Days[tm_time->tm_mon] < time;tm_time->tm_mon++)
    {
        time -= Days[tm_time->tm_mon];
    }

    tm_time->tm_mday = (int)(time);

    // Always set daylight saving time to 0 (i.e DST not in effect), as we currently do not handle it.
    tm_time->tm_isdst = 0;
    return;
}

} // namespace octo::logger::compat
