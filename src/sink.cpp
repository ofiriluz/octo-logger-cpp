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
#include <fmt/format.h>
#include <iomanip>
#include <thread>
#include <unistd.h>

namespace octo::logger
{
std::string Sink::formatted_log(Log const& log, Channel const& channel) const
{
    char dtf[1024];
    std::stringstream ss;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log.time_created().time_since_epoch());
    std::time_t time = std::chrono::duration_cast<std::chrono::seconds>(ms).count();
    auto fraction = ms.count() % 1000;
    std::strftime(dtf, sizeof(dtf), "[%d/%m/%Y %H:%M:%S", std::localtime(&time));
    std::string extra_id;
    if (!log.extra_identifier().empty())
    {
        extra_id = "[" + log.extra_identifier() + "]";
    }

    ss << dtf << "." << std::setfill('0') << std::setw(3) << fraction << "]["
       << Log::level_to_string(log.log_level())[0] << "][" << channel.channel_name() << "][PID(" << getpid()
       << ")][TID(" << std::this_thread::get_id() << ")]" << extra_id << ": " << log.stream()->str();

    return ss.str();
}

std::string Sink::formatted_context_info(Log const& log,
                                         Channel const& channel,
                                         Logger::ContextInfo const& context_info) const
{
    std::string context_info_str("context_info: ");
    for (auto const& itr : context_info)
    {
        context_info_str += fmt::format(FMT_STRING("[{:s}:{:s}]"), itr.first.data(), itr.second);
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

Sink::Sink(const SinkConfig& config) : config_(config)
{
}
} // namespace octo::logger
