
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
#ifdef _WIN32
    errno = localtime_s(result, timep);
    return errno == 0 ? result : nullptr;
#else
    return localtime_r(timep, result);
#endif
}

} // namespace octo::logger::compat


#endif // COMPAT_HPP
