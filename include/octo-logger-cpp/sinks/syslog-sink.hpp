/**
 * @file syslog-sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SYSLOG_SINK_HPP_
#define SYSLOG_SINK_HPP_

#ifndef _WIN32

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <syslog.h>
#include <unordered_map>

namespace octo::logger
{
class SysLogSink : public Sink
{
  private:
    std::string sys_log_name_;

  public:
    explicit SysLogSink(const SinkConfig& config);
    ~SysLogSink() override = default;

    void dump(const Log& log,
              const Channel& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override;
};
} // namespace octo::logger

#endif
#endif