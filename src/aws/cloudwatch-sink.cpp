/**
 * @file cloudwatch-sink.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifdef WITH_AWS

#include "octo-logger-cpp/aws/cloudwatch-sink.hpp"
#include <aws/logs/model/CreateLogGroupRequest.h>
#include <aws/logs/model/CreateLogStreamRequest.h>
#include <aws/logs/model/DescribeLogGroupsRequest.h>
#include <aws/logs/model/DescribeLogStreamsRequest.h>
#include <aws/logs/model/PutLogEventsRequest.h>
#include <fmt/format.h>
#include <iomanip>
#include <stdexcept>
#include <thread>

namespace octo::logger::aws
{
void CloudWatchSink::assert_and_create_log_group()
{
    Aws::CloudWatchLogs::Model::DescribeLogGroupsRequest describe_log_grps;
    describe_log_grps.SetLogGroupNamePrefix(log_group_name_.c_str());
    auto desc_outcome = aws_cloudwatch_client_->DescribeLogGroups(describe_log_grps);
    if (desc_outcome.IsSuccess())
    {
        bool found = false;
        for (auto& grp : desc_outcome.GetResult().GetLogGroups())
        {
            if (grp.GetLogGroupName() == log_group_name_)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Aws::CloudWatchLogs::Model::CreateLogGroupRequest create_log_grp;
            create_log_grp.SetLogGroupName(log_group_name_.c_str());
            if (!log_group_tags_.empty())
            {
                create_log_grp.SetTags(log_group_tags_);
            }
            auto create_outcome = aws_cloudwatch_client_->CreateLogGroup(create_log_grp);
        }
    }
}

void CloudWatchSink::assert_and_create_log_stream(const std::string& log_stream_name)
{
    if (std::find(existing_log_streams_.begin(), existing_log_streams_.end(), log_stream_name) ==
        existing_log_streams_.end())
    {
        Aws::CloudWatchLogs::Model::CreateLogStreamRequest create_log_stream;
        create_log_stream.WithLogGroupName(log_group_name_.c_str()).WithLogStreamName(log_stream_name.c_str());
        auto outcome = aws_cloudwatch_client_->CreateLogStream(create_log_stream);
        if (outcome.IsSuccess())
        {
            existing_log_streams_.insert(log_stream_name);
        }
    }
}

std::set<std::string> CloudWatchSink::list_existing_log_streams()
{
    Aws::CloudWatchLogs::Model::DescribeLogStreamsRequest describe_log_streams;
    describe_log_streams.SetLogGroupName(log_group_name_.c_str());
    auto outcome = aws_cloudwatch_client_->DescribeLogStreams(describe_log_streams);
    std::set<std::string> log_streams;
    if (outcome.IsSuccess())
    {
        for (auto& stream : outcome.GetResult().GetLogStreams())
        {
            log_streams.insert(stream.GetLogStreamName().c_str());
            if (!stream.GetUploadSequenceToken().empty())
            {
                sequence_tokens_[stream.GetLogStreamName()] = stream.GetUploadSequenceToken();
            }
        }
    }
    return log_streams;
}

void CloudWatchSink::send_log(CloudWatchLog&& log)
{
    Aws::CloudWatchLogs::Model::PutLogEventsRequest put_req;

    assert_and_create_log_stream(log.stream_name);

    put_req.WithLogGroupName(log_group_name_.c_str()).WithLogStreamName(log.stream_name.c_str());
    put_req.AddLogEvents(std::move(log.log_event));

    if (sequence_tokens_.find(log.stream_name) != sequence_tokens_.end())
    {
        put_req.WithSequenceToken(sequence_tokens_[log.stream_name].c_str());
    }
    auto outcome = aws_cloudwatch_client_->PutLogEvents(put_req);
    if (outcome.IsSuccess())
    {
        if (!outcome.GetResult().GetNextSequenceToken().empty())
        {
            std::lock_guard<std::mutex> lock(sequence_tokens_mtx_);
            sequence_tokens_[log.stream_name] = outcome.GetResult().GetNextSequenceToken();
        }
    }
}

void CloudWatchSink::cloudwatch_logs_thread()
{
    // Create the log group if it does not exist
    assert_and_create_log_group();
    existing_log_streams_ = list_existing_log_streams();
    while (is_running_)
    {
        std::unique_lock<std::mutex> unq_lk(logs_mtx_);
        try
        {
            logs_cond_.wait(unq_lk, [&]() { return !logs_queue_.empty() || !is_running_; });
            if (!is_running_)
            {
                break;
            }
            CloudWatchLog log = logs_queue_.front();
            logs_queue_.pop_front();
            send_log(std::move(log));
        }
        catch (const std::exception& e)
        {
            // Ignored, just so the thread itself will not die
        }
    }
    // Send the remainder logs
    while (!logs_queue_.empty())
    {
        try
        {
            CloudWatchLog log = logs_queue_.front();
            logs_queue_.pop_front();
            send_log(std::move(log));
        }
        catch (const std::exception& e)
        {
            // Ignored, just so the thread itself will not die
        }
    }
}

CloudWatchSink::CloudWatchSink(SinkConfig const& config,
                               std::string origin,
                               LogStreamType log_stream_type,
                               bool include_date_on_log_stream,
                               std::string const& log_group_name,
                               LogGroupTags log_group_tags)
    : Sink(config),
      log_stream_type_(log_stream_type),
      include_date_on_log_stream_(include_date_on_log_stream),
      origin_(std::move(origin)),
      log_group_name_(log_group_name),
      is_running_(true),
      log_group_tags_(std::move(log_group_tags))
{
#ifndef UNIT_TESTS
    // Create the AWS client
    aws_cloudwatch_client_ = Aws::MakeUnique<Aws::CloudWatchLogs::CloudWatchLogsClient>("cloudwatch");

    // Create the cloudwatch logs queue thread
    cloudwatch_logs_thread_ = std::make_unique<std::thread>(&CloudWatchSink::cloudwatch_logs_thread, this);
#endif
}

CloudWatchSink::~CloudWatchSink()
{
    {
        // std::lock_guard<std::mutex> lck(logs_mtx_);
        is_running_ = false;
        logs_cond_.notify_one();
    }
    if (cloudwatch_logs_thread_)
    {
        cloudwatch_logs_thread_->join();
        cloudwatch_logs_thread_.reset();
    }
}

std::string CloudWatchSink::log_stream_name(const Log& log, const Channel& channel) const
{
    switch (log_stream_type_)
    {
        case LogStreamType::BY_CHANNEL:
            return channel.channel_name();
        case LogStreamType::BY_EXTRA_ID:
        {
            if (!log.extra_identifier().empty())
            {
                return log.extra_identifier();
            }
            return origin_;
        }
        case LogStreamType::BY_BOTH:
        {
            if (!log.extra_identifier().empty())
            {
                return fmt::format("{}|{}", channel.channel_name(), log.extra_identifier());
            }
            return channel.channel_name();
        }
    }
    return channel.channel_name();
}

std::string CloudWatchSink::formatted_json(Log const& log,
                                           Channel const& channel,
                                           Logger::ContextInfo const& context_info) const
{
    nlohmann::json j;
    std::stringstream ss;
    std::time_t log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
    ss << std::put_time(std::localtime(&log_time_t), "%FT%T%z");
    j["message"] = log.stream()->str();
    j["origin"] = origin_;
    j["origin_service_name"] = channel.channel_name();
    j["timestamp"] = ss.str(); // ISO 8601
    auto log_level_str = Log::level_to_string(log.log_level());
    std::transform(log_level_str.begin(), log_level_str.end(), log_level_str.begin(), ::toupper);
    j["log_level"] = log_level_str;
    j["origin_func_name"] = "";

    j["context_info"] = init_context_info(log, channel, context_info);

    return j.dump();
}

void CloudWatchSink::init_context_info(nlohmann::json& dst,
                                       Log const& log,
                                       Channel const& channel,
                                       Logger::ContextInfo const& context_info) const
{
    switch (dst.type())
    {
        case nlohmann::json::value_t::null:
            dst = nlohmann::json::object();
        case nlohmann::json::value_t::object:
            break;
        default:
            throw std::runtime_error(fmt::format("Wrong context_info destination type {}", dst.type_name()));
    }

    for (auto const& itr : context_info)
    {
        if (!dst.contains(itr.first))
        {
            dst[itr.first.data()] = itr.second;
        }
    }

    if (!log.extra_identifier().empty())
    {
        dst["session_id"] = log.extra_identifier();
    }
}

nlohmann::json CloudWatchSink::init_context_info(Log const& log,
                                                 Channel const& channel,
                                                 Logger::ContextInfo const& context_info) const
{
    nlohmann::json j(nlohmann::json::value_t::object);
    init_context_info(j, log, channel, context_info);
    return std::move(j);
}

void CloudWatchSink::dump(Log const& log, Channel const& channel, Logger::ContextInfo const& context_info)
{
    try
    {
        Aws::CloudWatchLogs::Model::InputLogEvent e;
        // Set the event and add it to the queue
        e.WithTimestamp(Aws::Utils::DateTime(log.time_created()).Millis())
            .WithMessage(formatted_json(log, channel, context_info).c_str());
        logs_queue_.push_back(CloudWatchLog{std::move(e), std::move(log_stream_name(log, channel))});
        logs_cond_.notify_one();
    }
    catch (const std::exception& e)
    {
        // Ignored, just so the thread itself will not die
    }
}
} // namespace octo::logger::aws

#endif
