/**
 * @file sink.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/sink.hpp"

#include "octo-logger-cpp/compat.hpp"
#include "octo-logger-cpp/log-level.hpp"
#include <fmt/format.h>
#include <iomanip>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define getpid GetCurrentProcessId
#endif

namespace octo::logger
{
std::string Sink::formatted_log_plaintext_long(Log const& log,
                                               Channel const& channel,
                                               ContextInfo const& context_info,
                                               ContextInfo const& global_context_info,
                                               bool disable_context_info) const
{
    char dtf[1024];
    std::stringstream ss;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch());
    std::time_t time = std::chrono::duration_cast<std::chrono::seconds>(ms).count();
    auto fraction = ms.count() % 1000;
    struct tm timeinfo;
    std::strftime(dtf, sizeof(dtf), "[%d/%m/%Y %H:%M:%S", compat::localtime(&time, &timeinfo, safe_localtime_utc_));
    std::string extra_id;
    if (!log.extra_identifier().empty())
    {
        extra_id = "[" + log.extra_identifier() + "]";
    }

    ss << dtf << "." << std::setfill('0') << std::setw(3) << fraction << "]["
       << LogLevelUtils::level_to_string_short(log.log_level()) << "][" << channel.channel_name() << "][PID("
       << getpid() << ")][TID(" << std::this_thread::get_id() << ")]" << extra_id << ": " << log.str();

    if (!disable_context_info && !(context_info.empty() && log.context_info().empty() && global_context_info.empty()))
    {
        ss << "\n" << formatted_context_info(log, channel, context_info, global_context_info);
    }

    return ss.str();
}

#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
static void init_context_info_impl(nlohmann::json& dst,
                                   Log const& log,
                                   Channel const&,
                                   ContextInfo const& context_info,
                                   ContextInfo const& global_context_info)
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
}

static nlohmann::json init_context_info_impl(Log const& log,
                                             Channel const& channel,
                                             ContextInfo const& context_info,
                                             ContextInfo const& global_context_info)
{
    nlohmann::json j(nlohmann::json::value_t::object);
    init_context_info_impl(j, log, channel, context_info, global_context_info);
    return std::move(j);
}

nlohmann::json Sink::construct_log_json(Log const& log,
                                        Channel const& channel,
                                        ContextInfo const& context_info,
                                        ContextInfo const& global_context_info) const
{
    nlohmann::json j;
    std::stringstream ss;
    std::time_t const log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
    struct tm timeinfo;
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch()) % 1000;
    compat::localtime(&log_time_t, &timeinfo, safe_localtime_utc_);
    // Put datetime with milliseconds: YYYY-MM-DDTHH:MM:SS.mmm
    ss << std::put_time(&timeinfo, "%FT%T");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    // Put timezone as offset from UTC: ±HHMM
    ss << std::put_time(&timeinfo, "%z");


    j["message"] = log.str();
    j["origin"] = origin_;
    j["origin_service_name"] = channel.channel_name();
    j["timestamp"] = ss.str(); // ISO 8601
    j["log_level"] = LogLevelUtils::level_to_string_upper(log.log_level());
    j["origin_func_name"] = "";

    j["context_info"] = init_context_info_impl(log, channel, context_info, global_context_info);

    return j;
}

std::string Sink::formatted_log_json(Log const& log,
                                     Channel const& channel,
                                     ContextInfo const& context_info,
                                     ContextInfo const& global_context_info) const
{
    return construct_log_json(log, channel, context_info, global_context_info).dump();
}
#endif

std::string Sink::formatted_log_plaintext_short(Log const& log, Channel const& channel) const
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
       << LogLevelUtils::level_to_string_short(log.log_level()) << "][" << channel.channel_name() << "][TID("
       << std::this_thread::get_id() << ")]" << extra_id << ": " << log.str();
    return ss.str();
}

std::string Sink::formatted_context_info(Log const& log,
                                         Channel const& channel,
                                         ContextInfo const& context_info,
                                         ContextInfo const& global_context_info) const
{
    std::string context_info_str("context_info: ");
    // Note that if the same key is present in multiple context_infos, it will be logged multiple times
    for (auto const& ci_itr : {log.context_info(), context_info, global_context_info})
    {
        for (auto const& [key, value] : ci_itr)
        {
            context_info_str += fmt::format(FMT_STRING("[{:s}:{:s}]"), key.data(), value);
        }
    }
    return std::move(context_info_str);
}

std::string Sink::formatted_log(Log const& log,
                                Channel const& channel,
                                ContextInfo const& context_info,
                                ContextInfo const& global_context_info,
                                bool disable_context_info) const
{
    switch (line_format_)
    {
        case LineFormat::PLAINTEXT_LONG:
            return formatted_log_plaintext_long(log, channel, context_info, global_context_info, disable_context_info);
        case LineFormat::PLAINTEXT_SHORT:
            return formatted_log_plaintext_short(log, channel);
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
        case LineFormat::JSON:
        {
            try
            {
                return formatted_log_json(log, channel, context_info, global_context_info);
            }
            catch (nlohmann::json::exception const&)
            {
                // Fallback to default upon exception
                return formatted_log_plaintext_long(
                    log, channel, context_info, global_context_info, disable_context_info);
            }
            catch (std::exception const&)
            {
                // Fallback to default upon exception
                return formatted_log_plaintext_long(
                    log, channel, context_info, global_context_info, disable_context_info);
            }
        }
#endif
    }
    // Log an error only once so that it doesn't spam upon every log message
    static bool error_logged = false;
    if (!error_logged)
    {
        std::cerr << fmt::format("Unexpected Error Occurred - received illegal line format: [{}] falling back to PLAINTEXT_LONG", static_cast<int>(line_format_)) << std::endl;
        error_logged = true;
    }
    // Fallback to PLAINTEXT_LONG format
    return formatted_log_plaintext_long(
        log, channel, context_info, global_context_info, disable_context_info);
}

const SinkConfig& Sink::config() const
{
    return config_;
}

const std::string& Sink::sink_name() const
{
    return config_.sink_name();
}

void Sink::stop(bool discard)
{
    is_discarding_ = discard;
    stop_impl();
}

Sink::Sink(const SinkConfig& config, std::string const& origin, LineFormat format)
    : config_(config), is_discarding_(false), origin_(origin), line_format_(format), 
safe_localtime_utc_(config.option_default(SinkConfig::SinkOption::USE_SAFE_LOCALTIME_UTC, false))
{
}
} // namespace octo::logger
