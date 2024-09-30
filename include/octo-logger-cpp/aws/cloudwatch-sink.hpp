/**
 * @file cloudwatch-sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifdef OCTO_LOGGER_WITH_AWS

#ifndef CLOUDWATCH_SINK_HPP_
#define CLOUDWATCH_SINK_HPP_

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/fork-safe-mutex.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger-test-definitions.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/logs/CloudWatchLogsClient.h>
#include <aws/logs/model/InputLogEvent.h>
#include <nlohmann/json.hpp>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace octo::logger::aws
{
constexpr const auto AWS_LAMBDA_LOG_STREAM_NAME_ENV_VAR = "AWS_LAMBDA_LOG_STREAM_NAME";
constexpr const auto AWS_LAMBDA_LOG_GROUP_NAME_ENV_VAR = "AWS_LAMBDA_LOG_GROUP_NAME";

class CloudWatchSink : public Sink
{
  private:
    struct CloudWatchLog
    {
        Aws::CloudWatchLogs::Model::InputLogEvent log_event;
        std::string stream_name;
    };
    struct LogEventCmp
    {
        bool operator()(CloudWatchLog const& lhs, CloudWatchLog const& rhs) const
        {
            return lhs.log_event.GetTimestamp() < rhs.log_event.GetTimestamp();
        }
    };
    using AwsLogEventVector = Aws::Vector<Aws::CloudWatchLogs::Model::InputLogEvent>;
    using LogEventsMap = std::unordered_map<std::string, AwsLogEventVector>;

  public:
    enum class LogStreamType : std::uint8_t
    {
        BY_CHANNEL,
        BY_EXTRA_ID,
        BY_BOTH
    };
    typedef Aws::Map<Aws::String, Aws::String> LogGroupTags;

    static constexpr const auto DEFAULT_LOG_STREAM_TYPE = LogStreamType::BY_EXTRA_ID;
    static constexpr const auto DEFAULT_INCLUDE_DATE_ON_LOG_STREAM = true;
    static constexpr const auto DEFAULT_LOG_GROUP_NAME = "/octo";

    static constexpr auto ObservabilityLogsTag = "observability_logs";
    static constexpr auto ObservabilityLogsDisable = "false";
    static constexpr auto ObservabilityLogsEnable = "true";

  private:
    std::unordered_map<std::string, std::string> sequence_tokens_;
    Aws::UniquePtr<Aws::CloudWatchLogs::CloudWatchLogsClient> aws_cloudwatch_client_;
    std::deque<CloudWatchLog> logs_queue_;
    std::set<std::string> existing_log_streams_;
    LogStreamType log_stream_type_;
    bool include_date_on_log_stream_;
    std::atomic<bool> is_running_;
    std::string log_group_name_;
    std::unique_ptr<std::thread> cloudwatch_logs_thread_;
    std::condition_variable logs_cond_;
    ForkSafeMutex logs_mtx_, sequence_tokens_mtx_;
    LogGroupTags const log_group_tags_;
    pid_t thread_pid_;
    bool allow_overriding_by_aws_lambda_log_env_;

  private:
    bool using_aws_lambda_logging() const;
    void assert_and_create_log_stream(const std::string& log_stream_name);
    std::set<std::string> list_existing_log_streams();
    void assert_and_create_log_group();
    void send_logs(std::multiset<CloudWatchLog, LogEventCmp>&& logs) noexcept;
    bool send_log_events(std::string const& stream_name, AwsLogEventVector&& log_events) noexcept;
    std::string log_stream_name(const Log& log, const Channel& channel) const;
    std::string formatted_json(Log const& log,
                               Channel const& channel,
                               ContextInfo const& context_info,
                               ContextInfo const& global_context_info) const;
    void init_context_info(nlohmann::json& dst,
                           Log const& log,
                           Channel const& channel,
                           ContextInfo const& context_info,
                           ContextInfo const& global_context_info) const;
    nlohmann::json init_context_info(Log const& log,
                                     Channel const& channel,
                                     ContextInfo const& context_info,
                                     ContextInfo const& global_context_info) const;

    void report_logger_error(std::string_view message,
                             std::string const& name,
                             Aws::String const& error) const noexcept;
    void cloudwatch_logs_thread();
    bool started_by_current_process() const noexcept
    {
        return thread_pid_ == ::getpid();
    }

  protected:
    void stop_impl() override;

  public:
    CloudWatchSink(SinkConfig const& config,
                   std::string origin,
                   LogStreamType log_stream_type = DEFAULT_LOG_STREAM_TYPE,
                   bool include_date_on_log_stream = DEFAULT_INCLUDE_DATE_ON_LOG_STREAM,
                   std::string const& log_group_name = DEFAULT_LOG_GROUP_NAME,
                   LogGroupTags log_group_tags = {},
                   bool allow_overriding_by_aws_lambda_log_env = false);
    ~CloudWatchSink() override;

    void dump(Log const& log,
              Channel const& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override;
    void restart_sink() noexcept override;

    TESTS_MOCK_CLASS(CloudWatchSink)
};
} // namespace octo::logger::aws

#endif

#endif
