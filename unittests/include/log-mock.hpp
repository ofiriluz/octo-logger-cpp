#ifndef LOG_MOCK_HPP_
#define LOG_MOCK_HPP_

#include "octo-logger-cpp/log.hpp"
#include <chrono>
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
    LogMock(LogLevel const& log_level,
            std::string_view extra_identifier,
            ContextInfo&& context_info,
            Logger const& logger)
        : Log(log_level, std::move(extra_identifier), std::move(context_info), logger)
    {
        Log::time_created_ = std::chrono::system_clock::now();
    }

    std::optional<std::ostringstream> const& stream_wrapper() const
    {
        return Log::stream_;
    }

    std::optional<std::ostringstream>& stream_wrapper()
    {
        return Log::stream_;
    }

    Logger const& logger_wrapper() const
    {
        return Log::logger_;
    }

    auto& time_created()
    {
        return Log::time_created_;
    }
};

} // namespace octo::logger

#endif // LOG_MOCK_HPP_
