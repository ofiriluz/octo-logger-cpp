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
#include "octo-logger-cpp/compat.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <fmt/format.h>
#include <iomanip>
#include <thread>
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
#include <nlohmann/json.hpp>
#endif
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define getpid GetCurrentProcessId
#endif

#if defined(OCTO_LOGGER_WITH_JSON_FORMATTING) && !defined(UNIT_TESTS)
// We always want to compile these functions, which expose our internals to the unit tests.
namespace octo::logger::unittests
{
void init_context_info(nlohmann::json& dst,
                       Log const& log,
                       Channel const& channel,
                       ContextInfo const& context_info,
                       ContextInfo const& global_context_info);
nlohmann::json init_context_info(Log const& log,
                                 Channel const& channel,
                                 ContextInfo const& context_info,
                                 ContextInfo const& global_context_info);
} // namespace octo::logger::unittests
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
    std::strftime(dtf, sizeof(dtf), "[%d/%m/%Y %H:%M:%S", compat::localtime(&time, &timeinfo));
    std::string extra_id;
    if (!log.extra_identifier().empty())
    {
        extra_id = "[" + log.extra_identifier() + "]";
    }

    ss << dtf << "." << std::setfill('0') << std::setw(3) << fraction << "]["
       << Log::level_to_string(log.log_level())[0] << "][" << channel.channel_name() << "][PID(" << getpid()
       << ")][TID(" << std::this_thread::get_id() << ")]" << extra_id << ": " << log.str();

    if (!disable_context_info && !(context_info.empty() && log.context_info().empty() && global_context_info.empty()))
    {
        ss << "\n" << formatted_context_info(log, channel, context_info, global_context_info);
    }

    return ss.str();
}

#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
static void init_context_info_impl(nlohmann::json& dst,
                                   Log const& log,
                                   Channel const& channel,
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

std::string Sink::formatted_log_json(Log const& log,
                                     Channel const& channel,
                                     ContextInfo const& context_info,
                                     ContextInfo const& global_context_info) const
{
    nlohmann::json j;
    std::stringstream ss;
    std::time_t const log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
    struct tm timeinfo;
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch()) % 1000;
    // Put datetime with milliseconds: YYYY-MM-DDTHH:MM:SS.mmm
    ss << std::put_time(compat::localtime(&log_time_t, &timeinfo), "%FT%T");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    // Put timezone as offset from UTC: ±HHMM
    ss << std::put_time(compat::localtime(&log_time_t, &timeinfo), "%z");
    j["message"] = log.str();
    j["origin"] = origin_;
    j["origin_service_name"] = channel.channel_name();
    j["timestamp"] = ss.str(); // ISO 8601
    auto log_level_str = Log::level_to_string(log.log_level());
    std::transform(log_level_str.begin(), log_level_str.end(), log_level_str.begin(), ::toupper);
    j["log_level"] = log_level_str;
    j["origin_func_name"] = "";

    j["context_info"] = init_context_info_impl(log, channel, context_info, global_context_info);

    return j.dump();
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
    ss << "[MS(" << std::setfill('0') << std::setw(3) << fraction << ")][" << Log::level_to_string(log.log_level())[0]
       << "][" << channel.channel_name() << "][TID(" << std::this_thread::get_id() << ")]" << extra_id << ": "
       << log.str();
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
            return formatted_log_plaintext_long(
                log, channel, context_info, global_context_info, disable_context_info);
        case LineFormat::PLAINTEXT_SHORT:
            return formatted_log_plaintext_short(log, channel);
#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING
        case LineFormat::JSON:
        {
            try
            {
                return formatted_log_json(log, channel, context_info, global_context_info);
            }
            catch(const nlohmann::json::exception & ex)
            {
                // Fallback to default upon exception
                return formatted_log_plaintext_long(
                    log, channel, context_info, global_context_info, disable_context_info);
            }
            catch(const std::exception & ex)
            {
                // Fallback to default upon exception
                return formatted_log_plaintext_long(
                    log, channel, context_info, global_context_info, disable_context_info);
            }
        }
#endif
    }
    throw std::runtime_error("Unexpected Error Occurred");
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
    : config_(config), origin_(std::move(origin)), line_format_(format), is_discarding_(false)
{
}
} // namespace octo::logger
