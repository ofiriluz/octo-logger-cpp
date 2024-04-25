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

inline struct tm* localtime(const time_t* timep, struct tm* result)
{
#ifndef _WIN32
    return localtime_r(timep, result);
#else
    errno = localtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#endif
}

} // namespace octo::logger::compat


#endif // COMPAT_HPP
