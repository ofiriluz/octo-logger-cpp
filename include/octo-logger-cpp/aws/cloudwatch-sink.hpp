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
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger-test-definitions.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/logs/CloudWatchLogsClient.h>
#include <aws/logs/model/InputLogEvent.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <map>
#include <thread>
#include <deque>
#include <set>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace octo::logger::aws
{
class CloudWatchSink : public Sink
{
  private:
    struct CloudWatchLog
    {
        Aws::CloudWatchLogs::Model::InputLogEvent log_event;
        std::string stream_name;
    };

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
    bool is_running_;
    std::string log_group_name_;
    std::unique_ptr<std::thread> cloudwatch_logs_thread_;
    std::condition_variable logs_cond_;
    std::mutex logs_mtx_, sequence_tokens_mtx_;
    LogGroupTags const log_group_tags_;

  private:
    void assert_and_create_log_stream(const std::string& log_stream_name);
    std::set<std::string> list_existing_log_streams();
    void assert_and_create_log_group();
    void send_log(CloudWatchLog&& log);
    std::string log_stream_name(const Log& log, const Channel& channel) const;
    void cloudwatch_logs_thread();

  public:
    CloudWatchSink(SinkConfig const& config,
                   std::string origin,
                   LogStreamType log_stream_type = DEFAULT_LOG_STREAM_TYPE,
                   bool include_date_on_log_stream = DEFAULT_INCLUDE_DATE_ON_LOG_STREAM,
                   std::string const& log_group_name = DEFAULT_LOG_GROUP_NAME,
                   LogGroupTags log_group_tags = {});
    ~CloudWatchSink() override;

    void dump(Log const& log, Channel const& channel, Logger::ContextInfo const& context_info) override;

    TESTS_MOCK_CLASS(CloudWatchSink)
};
} // namespace octo::logger::aws

#endif

#endif