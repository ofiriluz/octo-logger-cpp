#ifdef OCTO_LOGGER_WITH_AWS
#ifndef CLOUDWATCH_SINK_MOCK_HPP_
#define CLOUDWATCH_SINK_MOCK_HPP_

#include "octo-logger-cpp/aws/cloudwatch-sink.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include <memory>
#include <string>

namespace octo::logger::unittests
{
using CloudWatchSinkMock = octo::logger::aws::CloudWatchSink::CloudWatchSinkMock;
}

namespace octo::logger::aws
{
class CloudWatchSink::CloudWatchSinkMock : public CloudWatchSink
{
  public:
    CloudWatchSinkMock(SinkConfig const& config,
                       std::string origin,
                       LogStreamType log_stream_type = DEFAULT_LOG_STREAM_TYPE,
                       bool include_date_on_log_stream = DEFAULT_INCLUDE_DATE_ON_LOG_STREAM,
                       std::string const& log_group_name = DEFAULT_LOG_GROUP_NAME,
                       LogGroupTags log_group_tags = {})
        : CloudWatchSink(config,
                         std::move(origin),
                         log_stream_type,
                         include_date_on_log_stream,
                         log_group_name,
                         std::move(log_group_tags))
    {
    }

    LogStreamType log_stream_type_getter() const
    {
        return CloudWatchSink::log_stream_type_;
    }

    bool include_date_on_log_stream_getter() const
    {
        return CloudWatchSink::include_date_on_log_stream_;
    }

    std::string const& origin_getter() const
    {
        return CloudWatchSink::origin_;
    }

    std::string const& log_group_name_getter() const
    {
        return CloudWatchSink::log_group_name_;
    }

    bool is_running_getter() const
    {
        return CloudWatchSink::is_running_;
    }

    std::string formatted_json_wrapper(Log const& log,
                                       Channel const& channel,
                                       Logger::ContextInfo const& context_info) const
    {
        return CloudWatchSink::formatted_json(log, channel, context_info);
    }

    void init_context_info_wrapper(nlohmann::json& dst,
                                   Log const& log,
                                   Channel const& channel,
                                   Logger::ContextInfo const& context_info) const
    {
        return CloudWatchSink::init_context_info(dst, log, channel, context_info);
    }

    nlohmann::json init_context_info_wrapper(Log const& log,
                                             Channel const& channel,
                                             Logger::ContextInfo const& context_info) const
    {
        return CloudWatchSink::init_context_info(log, channel, context_info);
    }
};

} // namespace octo::logger::aws::unittests

#endif // CLOUDWATCH_SINK_MOCK_HPP_
#endif
