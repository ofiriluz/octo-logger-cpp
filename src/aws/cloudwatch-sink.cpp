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

#ifdef OCTO_LOGGER_WITH_AWS

#include "octo-logger-cpp/aws/cloudwatch-sink.hpp"

#include "octo-logger-cpp/compat.hpp"
#include "octo-logger-cpp/log-level.hpp"
#include <aws/logs/model/CreateLogGroupRequest.h>
#include <aws/logs/model/CreateLogStreamRequest.h>
#include <aws/logs/model/DescribeLogGroupsRequest.h>
#include <aws/logs/model/DescribeLogStreamsRequest.h>
#include <aws/logs/model/PutLogEventsRequest.h>
#include <fmt/format.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <thread>

#define HANDLE_CLOUDWATCH_SINK_ERROR(_err, _action, _ret)                                                              \
    if (_err.has_value())                                                                                              \
    {                                                                                                                  \
        service_logger_.error().formatted(FMT_STRING("Failed to {:s} [{:s}] service"), #_action, _err->formatted());   \
        _ret                                                                                                           \
    }

namespace
{
constexpr auto THREAD_WAIT_DURATION = std::chrono::milliseconds(500);
constexpr std::size_t AWS_LOGS_PER_REQUEST_LIMIT = 50;
} // namespace

namespace octo::logger::aws
{
bool CloudWatchSink::using_aws_lambda_logging() const
{
    return allow_overriding_by_aws_lambda_log_env_ && std::getenv(AWS_LAMBDA_LOG_GROUP_NAME_ENV_VAR) &&
           std::getenv(AWS_LAMBDA_LOG_STREAM_NAME_ENV_VAR);
}

void CloudWatchSink::assert_and_create_log_group()
{
    if (using_aws_lambda_logging())
    {
        return;
    }
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
            if (!create_outcome.IsSuccess())
            {
                report_logger_error(
                    "failed to create log group", log_group_name_, create_outcome.GetError().GetMessage());
            }
        }
    }
    else
    {
        report_logger_error("failed to describe log group", log_group_name_, desc_outcome.GetError().GetMessage());
    }
}

void CloudWatchSink::assert_and_create_log_stream(const std::string& log_stream_name)
{
    if (using_aws_lambda_logging())
    {
        return;
    }
    if (std::find(existing_log_streams_.begin(), existing_log_streams_.end(), log_stream_name) ==
        existing_log_streams_.end())
    {
        Aws::CloudWatchLogs::Model::DescribeLogStreamsRequest describe_log_streams;
        describe_log_streams.SetLogGroupName(log_group_name_.c_str());
        describe_log_streams.SetLogStreamNamePrefix(log_stream_name.c_str());
        auto const describe_outcome = aws_cloudwatch_client_->DescribeLogStreams(describe_log_streams);
        if (describe_outcome.IsSuccess())
        {
            const auto& log_streams = describe_outcome.GetResult().GetLogStreams();
            for (const auto& log_stream : log_streams)
            {
                if (log_stream.GetLogStreamName() == log_stream_name)
                {
                    existing_log_streams_.insert(log_stream_name);
                    return;
                }
            }
        }
        Aws::CloudWatchLogs::Model::CreateLogStreamRequest create_log_stream;
        create_log_stream.WithLogGroupName(log_group_name_.c_str()).WithLogStreamName(log_stream_name.c_str());
        auto const outcome = aws_cloudwatch_client_->CreateLogStream(create_log_stream);
        if (outcome.IsSuccess())
        {
            existing_log_streams_.insert(log_stream_name);
        }
        else if (outcome.GetError().GetMessage() == "The specified log stream already exists")
        {
            // LogStream was already created, most likely in the main process, and we are in the child process.
            existing_log_streams_.insert(log_stream_name);
        }
        else
        {
            report_logger_error("failed to create stream", log_stream_name, outcome.GetError().GetMessage());
        }
    }
}

std::set<std::string> CloudWatchSink::list_existing_log_streams()
{
    if (using_aws_lambda_logging())
    {
        return {std::getenv(AWS_LAMBDA_LOG_STREAM_NAME_ENV_VAR)};
    }
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
    else
    {
        report_logger_error("failed to describe streams", log_group_name_, outcome.GetError().GetMessage());
    }
    return log_streams;
}

void CloudWatchSink::send_logs(std::multiset<CloudWatchLog, LogEventCmp>&& logs) noexcept
{
    if (is_discarding())
    {
        return;
    }

    LogEventsMap log_events_map;
    std::for_each(logs.cbegin(), logs.cend(), [&log_events_map](CloudWatchLog const& itr) -> void {
        log_events_map[itr.stream_name].push_back(std::move(itr.log_event));
    });

    std::for_each(log_events_map.begin(), log_events_map.end(), [&](LogEventsMap::value_type& itr) -> void {
        send_log_events(itr.first, std::move(itr.second));
    });
}

bool CloudWatchSink::send_log_events(std::string const& stream_name, AwsLogEventVector&& log_events) noexcept
{
    std::size_t const log_event_count = log_events.size();
    Aws::CloudWatchLogs::Model::PutLogEventsRequest put_req;

    assert_and_create_log_stream(stream_name);

    put_req.WithLogGroupName(log_group_name_.c_str()).WithLogStreamName(stream_name.c_str());
    put_req.SetLogEvents(std::move(log_events));

    auto const& sequence_token = sequence_tokens_.find(stream_name);
    if (sequence_token != sequence_tokens_.cend())
    {
        put_req.WithSequenceToken(sequence_token->second.c_str());
    }
    Aws::CloudWatchLogs::Model::PutLogEventsOutcome outcome;
    try
    {
        outcome = aws_cloudwatch_client_->PutLogEvents(put_req);
    }
    catch (std::exception const& e)
    {
        report_logger_error(
            fmt::format(FMT_STRING("Encountered an error while sending [{:d}] log events"), log_event_count),
            stream_name,
            e.what());
        return false;
    }
    if (!outcome.IsSuccess())
    {
        report_logger_error(fmt::format(FMT_STRING("Failed to put [{:d}] log events"), log_event_count),
                            stream_name,
                            outcome.GetError().GetMessage());
        return false;
    }
    if (!outcome.GetResult().GetNextSequenceToken().empty())
    {
        std::lock_guard<std::mutex> lock(sequence_tokens_mtx_.get());
        sequence_tokens_[stream_name] = outcome.GetResult().GetNextSequenceToken();
    }
    return true;
}

void CloudWatchSink::cloudwatch_logs_thread()
{
    // Create the log group if it does not exist
    assert_and_create_log_group();
    existing_log_streams_ = list_existing_log_streams();
    while (is_running_)
    {
        try
        {
            std::unique_lock<std::mutex> lock(logs_mtx_.get());
            logs_cond_.wait_for(lock, THREAD_WAIT_DURATION, [&]() -> bool {
                return logs_queue_.size() >= AWS_LOGS_PER_REQUEST_LIMIT || !is_running_;
            });
            if (!is_running_)
            {
                break;
            }
            if (logs_queue_.empty())
            {
                continue;
            }

            std::multiset<CloudWatchLog, LogEventCmp> logs;
            for (std::size_t i = 0; i < AWS_LOGS_PER_REQUEST_LIMIT && !logs_queue_.empty(); ++i)
            {
                logs.insert(std::move(logs_queue_.front()));
                logs_queue_.pop_front();
            }
            send_logs(std::move(logs));
        }
        catch (std::exception const& e)
        {
            // Ignored, just so the thread itself will not die
            report_logger_error("Encountered an error while sending log events", "", e.what());
        }
    }
    try
    {
        std::unique_lock<std::mutex> lock(logs_mtx_.get());
        // Send the remainder logs
        while (!logs_queue_.empty())
        {
            try
            {
                std::multiset<CloudWatchLog, LogEventCmp> logs;
                while (!logs_queue_.empty())
                {
                    for (std::size_t i = 0; i < AWS_LOGS_PER_REQUEST_LIMIT && !logs_queue_.empty(); ++i)
                    {
                        logs.insert(std::move(logs_queue_.front()));
                        logs_queue_.pop_front();
                    }
                    send_logs(std::move(logs));
                }
            }
            catch (std::exception const& e)
            {
                // Ignored, just so the thread itself will not die
                report_logger_error("Encountered an error while flushing remaining logs", "", e.what());
            }
        }
    }
    catch (std::exception const& e)
    {
        // Ignored, just so the thread itself will not die
        report_logger_error("Encountered an error while flushing remaining logs", "", e.what());
    }
}

CloudWatchSink::CloudWatchSink(SinkConfig const& config,
                               std::string origin,
                               LogStreamType log_stream_type,
                               bool include_date_on_log_stream,
                               std::string const& log_group_name,
                               LogGroupTags log_group_tags,
                               bool allow_overriding_by_aws_lambda_log_env,
                               bool log_thread_id)
    : Sink(config, std::move(origin), LineFormat::JSON),
      log_stream_type_(log_stream_type),
      include_date_on_log_stream_(include_date_on_log_stream),
      log_group_name_(log_group_name),
      is_running_(true),
      log_group_tags_(std::move(log_group_tags)),
      thread_pid_(::getpid()),
      allow_overriding_by_aws_lambda_log_env_(allow_overriding_by_aws_lambda_log_env),
      log_thread_id_(log_thread_id || config.option_default<bool>(SinkConfig::SinkOption::LOG_THREAD_ID, false))
{
#ifndef UNIT_TESTS
    // Create the AWS client
    aws_cloudwatch_client_ = Aws::MakeUnique<Aws::CloudWatchLogs::CloudWatchLogsClient>("cloudwatch");

    // Create the cloudwatch logs queue thread
    cloudwatch_logs_thread_ = std::make_unique<std::thread>(&CloudWatchSink::cloudwatch_logs_thread, this);
#endif
    if (using_aws_lambda_logging())
    {
        log_group_name_ = std::getenv(AWS_LAMBDA_LOG_GROUP_NAME_ENV_VAR);
    }
}

CloudWatchSink::~CloudWatchSink()
{
    stop_impl();
}

std::string CloudWatchSink::log_stream_name(const Log& log, const Channel& channel) const
{
    if (using_aws_lambda_logging())
    {
        return std::getenv(AWS_LAMBDA_LOG_STREAM_NAME_ENV_VAR);
    }
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
                                           ContextInfo const& context_info,
                                           ContextInfo const& global_context_info) const
{
    nlohmann::json j;
    std::stringstream ss;
    std::time_t const log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
    if (timestamp_format_ == Sink::TimestampFormat::ISO8601)
    {
        std::stringstream ss;
        std::time_t const log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
        struct tm timeinfo;
        auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch()) % 1000;
        // Put datetime with milliseconds: YYYY-MM-DDTHH:MM:SS.mmm
        ss << std::put_time(compat::localtime(&log_time_t, &timeinfo), "%FT%T");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        // Put timezone as offset from UTC: ±HHMM
        ss << std::put_time(compat::localtime(&log_time_t, &timeinfo), "%z");
        j["timestamp"] = ss.str(); // ISO 8601
    }
    else if (timestamp_format_ == Sink::TimestampFormat::UNIX_EPOCH)
    {
        auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch());
        
        j["timestamp"] =  static_cast<double>(ms.count()) / 1000.0;
    }
    else
    {
        throw std::runtime_error(fmt::format("Unexpected timestamp format: {}", static_cast<int>(timestamp_format_)));
    }
    j["message"] = log.str();
    j["origin"] = origin_;
    j["origin_service_name"] = channel.channel_name();
    j["log_level"] = LogLevelUtils::level_to_string_upper(log.log_level());
    j["origin_func_name"] = "";

    j["context_info"] = init_context_info(log, channel, context_info, global_context_info);

    return j.dump();
}

void CloudWatchSink::init_context_info(nlohmann::json& dst,
                                       Log const& log,
                                       [[maybe_unused]] Channel const& channel,
                                       ContextInfo const& context_info,
                                       ContextInfo const& global_context_info) const
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

    // This determines the precedence of the different contexts - the most local context_info has the highest precedence
    for (auto const& ci_itr : {log.context_info(), context_info, global_context_info})
    {
        for (auto const& [key, value] : ci_itr)
        {
            if (!dst.contains(key))
            {
                dst[key.data()] = value;
            }
        }
    }

    if (!log.extra_identifier().empty())
    {
        dst["session_id"] = log.extra_identifier();
    }

    if (log_thread_id_)
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        dst["thread_id"] = oss.str();
    }
}

void CloudWatchSink::report_logger_error(std::string_view message,
                                         std::string const& name,
                                         Aws::String const& error) const noexcept
{
    fmt::print(
        stderr, FMT_STRING("{}[pid={}]: {} [name={}] [error={}]\n"), sink_name(), getpid(), message, name, error);
}

nlohmann::json CloudWatchSink::init_context_info(Log const& log,
                                                 Channel const& channel,
                                                 ContextInfo const& context_info,
                                                 ContextInfo const& global_context_info) const
{
    nlohmann::json j(nlohmann::json::value_t::object);
    init_context_info(j, log, channel, context_info, global_context_info);
    return j;
}

void CloudWatchSink::dump(Log const& log,
                          Channel const& channel,
                          ContextInfo const& context_info,
                          ContextInfo const& global_context_info)
{
    if (!is_running_)
    {
        return;
    }
    try
    {
        Aws::CloudWatchLogs::Model::InputLogEvent e;
        auto message = formatted_json(log, channel, context_info, global_context_info);
        auto log_name = log_stream_name(log, channel);
        if (message.empty() || log_name.empty())
        {
            return;
        }
        // Set the event and add it to the queue
        e.WithTimestamp(Aws::Utils::DateTime(log.time_created()).Millis()).WithMessage(std::move(message));
        logs_queue_.push_back(CloudWatchLog{std::move(e), std::move(log_name)});
    }
    catch (const std::exception& e)
    {
        // Ignored, just so the thread itself will not die
    }
}

void CloudWatchSink::stop_impl()
{
    if (!is_running_)
    {
        return;
    }
    try
    {
        is_running_ = false;
        if (started_by_current_process() && cloudwatch_logs_thread_ && cloudwatch_logs_thread_->joinable())
        {
            cloudwatch_logs_thread_->join();
        }
        else if (!started_by_current_process())
        {
            // We are in a forked process, calling `reset()` will hang the client.
            cloudwatch_logs_thread_.release();
            aws_cloudwatch_client_.release();
            logs_mtx_.fork_reset();
            sequence_tokens_mtx_.fork_reset();
        }
        cloudwatch_logs_thread_.reset();
        aws_cloudwatch_client_.reset();
        logs_queue_.clear();
    }
    catch (const std::exception& e)
    {
        // Ignored, just so the thread itself will not die
    }
}

void CloudWatchSink::restart_sink() noexcept
{
    if (!is_running_)
    {
        report_logger_error("Requested to restart sink but sink is not running!", log_group_name_, "");
        return;
    }
    try
    {
        stop_impl();
        logs_queue_.clear();
        is_running_ = true;
        thread_pid_ = ::getpid();
        aws_cloudwatch_client_ = Aws::MakeUnique<Aws::CloudWatchLogs::CloudWatchLogsClient>("cloudwatch");
        cloudwatch_logs_thread_ = std::make_unique<std::thread>(&CloudWatchSink::cloudwatch_logs_thread, this);
    }
    catch (const std::exception& e)
    {
        // Ignored, just so the thread itself will not die
    }
}
} // namespace octo::logger::aws

#endif
