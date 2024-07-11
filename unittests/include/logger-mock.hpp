#ifndef LOGGER_MOCK_HPP_
#define LOGGER_MOCK_HPP_

#include "octo-logger-cpp/logger.hpp"
#include <string>
#include <string_view>

namespace octo::logger::unittests
{
using LoggerMock = octo::logger::Logger::LoggerMock;
}

namespace octo::logger
{
class Logger::LoggerMock : public Logger
{
  public:
    LoggerMock() : Logger("LoggerTest")
    {
    }
    explicit LoggerMock(std::string_view channel) : Logger(channel)
    {
    }

    std::string const& channel_name_getter() const
    {
        return Logger::channel_view_.channel().channel_name();
    }

    ContextInfo& context_info_getter()
    {
        return Logger::context_info_;
    }
};

} // namespace octo::logger

#endif // LOGGER_MOCK_HPP_
