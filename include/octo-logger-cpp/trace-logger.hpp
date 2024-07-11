#ifndef TRACE_LOGGER_HPP_
#define TRACE_LOGGER_HPP_

#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/manager.hpp"

#define METHOD_LOG_TRACE_GLOBAL                                                                                        \
    ::octo::logger::TraceLogger const _trace_logger(__FUNCTION__, ::octo::logger::Manager::instance().global_logger());

#define METHOD_LOG_TRACE ::octo::logger::TraceLogger const _trace_logger(__FUNCTION__, logger_);

namespace octo::logger
{

class TraceLogger
{
  private:
    const char* const function_;
    Logger const& logger_;

  public:
    TraceLogger(const char* function_name, Logger const& logger) : function_(function_name), logger_(logger)
    {
        logger_.trace() << "++" << function_;
    }

    ~TraceLogger()
    {
        logger_.trace() << "~~" << function_;
    }
};

} // namespace octo::logger

#endif // TRACE_LOGGER_HPP_
