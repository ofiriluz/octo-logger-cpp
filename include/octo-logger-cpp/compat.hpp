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
struct tm *gmtime_safe(time_t time, struct tm *tm_time);

struct tm* localtime(const time_t* timep, struct tm* result);

} // namespace octo::logger::compat

#endif // COMPAT_HPP