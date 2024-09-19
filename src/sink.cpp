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
void init_context_info(nlohmann::json& dst, Log const& log, Channel const& channel, ContextInfo const& context_info,ContextInfo const& global_context_info);
nlohmann::json init_context_info(Log const& log, Channel const& channel, ContextInfo const& context_info,ContextInfo const& global_context_info);
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
       << ")][TID(" << std::this_thread::get_id() << ")]" << extra_id << ": " << log.stream()->str();

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

    for (auto const& ci_itr : {log.context_info(), context_info, global_context_info})
    {
        for (auto const& itr : ci_itr)
        {
            if (!dst.contains(itr.first))
            {
                dst[itr.first.data()] = itr.second;
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

void octo::logger::unittests::init_context_info(nlohmann::json& dst,
                                                Log const& log,
                                                Channel const& channel,
                                                ContextInfo const& context_info,
                                                ContextInfo const& global_context_info)
{
    init_context_info_impl(dst, log, channel, context_info, global_context_info);
}

nlohmann::json octo::logger::unittests::init_context_info(Log const& log,
                                                          Channel const& channel,
                                                          ContextInfo const& context_info,
                                                          ContextInfo const& global_context_info)
{
    return init_context_info_impl(log, channel, context_info, global_context_info);
}

std::string Sink::formatted_log_json(Log const& log,
                                     Channel const& channel,
                                     ContextInfo const& context_info,
                                     ContextInfo const& global_context_info) const
{
    nlohmann::json j;
    std::stringstream ss;
    std::time_t log_time_t = std::chrono::system_clock::to_time_t(log.time_created());
    struct tm timeinfo;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch());
    auto fraction = ms.count() % 1000;
    ss << std::put_time(compat::localtime(&log_time_t, &timeinfo), "%FT%T%z");
    ss << "." << std::setfill('0') << std::setw(3) << fraction;
    j["message"] = log.stream()->str();
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
       << log.stream()->str();
    return ss.str();
}

std::string Sink::formatted_context_info(Log const& log,
                                         Channel const& channel,
                                         ContextInfo const& context_info,
                                         ContextInfo const& global_context_info) const
{
    std::string context_info_str("context_info: ");
    for (auto const& ci_itr : {log.context_info(), context_info, global_context_info})
    {
        for (auto const& itr : ci_itr)
        {
            context_info_str += fmt::format(FMT_STRING("[{:s}:{:s}]"), itr.first.data(), itr.second);
        }
    }
    return std::move(context_info_str);
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
