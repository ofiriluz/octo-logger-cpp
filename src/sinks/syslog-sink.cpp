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

#include "octo-logger-cpp/sinks/syslog-sink.hpp"
#include <iomanip>

namespace octo::logger
{
SysLogSink::SysLogSink(const SinkConfig& config) : Sink(config)
{
    sys_log_name_ = config.option_default(SinkConfig::SinkOption::SYSLOG_LOG_NAME, "octo-logger-cpp");
}

void SysLogSink::dump(const Log& log, const Channel& channel, Logger::ContextInfo const& context_info)
{
    if (log.stream())
    {
        std::stringstream ss;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch());
        auto fraction = ms.count() % 1000;
        std::string extra_id = "";
        if (!log.extra_identifier().empty())
        {
            extra_id = "[" + log.extra_identifier() + "]";
        }
        ss << "[MS(" << std::setfill('0') << std::setw(3) << fraction << ")]["
           << Log::level_to_string(log.log_level())[0] << "][" << channel.channel_name() << "][TID("
           << std::this_thread::get_id() << ")]" << extra_id << ": " << log.stream()->str();
        openlog(sys_log_name_.c_str(), LOG_PID | LOG_CONS, LOG_AUTHPRIV);
        syslog(LOG_INFO | LOG_AUTHPRIV, "%s", ss.str().c_str());
        closelog();
    }
}
} // namespace octo::logger
