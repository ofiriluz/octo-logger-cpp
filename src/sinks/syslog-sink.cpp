/**
 * @file syslog-sink.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _WIN32

#include "octo-logger-cpp/sinks/syslog-sink.hpp"
#include <iomanip>

namespace octo::logger
{
SysLogSink::SysLogSink(const SinkConfig& config)
    : Sink(config, "", extract_format_with_default(config, LineFormat::PLAINTEXT_SHORT))
{
    sys_log_name_ = config.option_default(SinkConfig::SinkOption::SYSLOG_LOG_NAME, "octo-logger-cpp");
}

void SysLogSink::dump(const Log& log,
                      const Channel& channel,
                      ContextInfo const& context_info,
                      ContextInfo const& global_context_info)
{
    if (log.has_stream())
    {
        std::string line{formatted_log(log, channel, context_info, global_context_info, false)};
        openlog(sys_log_name_.c_str(), LOG_PID | LOG_CONS, LOG_AUTHPRIV);
        syslog(LOG_INFO | LOG_AUTHPRIV, "%s", line.c_str());
        closelog();
    }
}
} // namespace octo::logger

#endif
