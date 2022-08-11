#ifndef LOG_MOCK_HPP_
#define LOG_MOCK_HPP_

#include "octo-logger-cpp/log.hpp"
#include <string>
#include <string_view>

namespace octo::logger::unittests
{
using LogMock = octo::logger::Log::LogMock;
}

namespace octo::logger
{
class Log::LogMock : public Log
{
  public:
    LogMock(LogLevel const& log_level, std::string_view extra_identifier, Logger const& logger)
        : Log(log_level, std::move(extra_identifier), logger)
    {
    }

    std::unique_ptr<std::ostringstream> const& stream_wrapper() const
    {
        return Log::stream_;
    }

    std::unique_ptr<std::ostringstream>& stream_wrapper()
    {
        return Log::stream_;
    }

    Logger const& logger_wrapper() const
    {
        return Log::logger_;
    }
};

} // namespace octo::logger

#endif // LOG_MOCK_HPP_
