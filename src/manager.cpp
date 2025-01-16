/**
 * @file manager.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/manager.hpp"

namespace octo::logger
{
std::shared_ptr<Manager> Manager::manager_;
std::mutex Manager::manager_init_mutex_;

Manager::Manager()
    : config_(std::make_shared<ManagerConfig>()),
      default_log_level_(Log::LogLevel::INFO),
      global_context_info_(std::make_shared<Manager::GlobalContextInfoType>())
{
}

Manager& Manager::instance()
{
    if (manager_)
    {
        return *manager_;
    }
    {
        std::lock_guard<std::mutex> lock(manager_init_mutex_);
        if (!manager_)
        {
            manager_ = std::shared_ptr<Manager>(new Manager());
            manager_->global_logger_ = std::make_shared<Logger>("GLOBAL");
        }
    }
    return *manager_;
}

void Manager::reset_manager()
{
    std::lock_guard<std::mutex> lock(manager_init_mutex_);
    manager_.reset();
}

Manager::~Manager()
{
    terminate();
}

ChannelView Manager::create_channel(std::string_view name)
{
    return ChannelView(
        channels_.try_emplace(name.data(), std::make_shared<Channel>(name, default_log_level_)).first->second);
}

const Channel& Manager::channel(const std::string& name) const
{
    if (channels_.find(name) != channels_.cend())
    {
        return *channels_.at(name);
    }
    throw std::runtime_error("No channel for given name [" + name + "]");
}

Channel& Manager::editable_channel(const std::string& name)
{
    if (channels_.find(name) != channels_.cend())
    {
        return *channels_.at(name);
    }
    throw std::runtime_error("No channel for given name [" + name + "]");
}

void Manager::configure(const ManagerConfigPtr& config, bool clear_old_sinks)
{
    if (clear_old_sinks)
    {
        clear_sinks();
    }
    config_ = config;

    // Change the default level if requested by config
    if (config_->has_option(ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL))
    {
        int default_level;
        if (config_->option(ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL, default_level))
        {
            default_log_level_ = static_cast<Log::LogLevel>(default_level);
        }
    }
    {
        std::lock_guard<std::mutex> lock(manager_init_mutex_);
        // Create all the sinks
        for (auto& sink_config : config_->sinks())
        {
            SinkPtr sink = SinkFactory::instance().create_sink(sink_config);
            if (sink)
            {
                sinks_.push_back(std::move(sink));
            }
        }
        for (auto& sink : config_->custom_sinks())
        {
            sinks_.push_back(sink);
        }
    }
    for (auto const& channel : channels_)
    {
        channel.second->set_log_level(default_log_level_);
    }
}

void Manager::terminate()
{
    stop(false);
    clear_sinks();
    clear_channels();
}

void Manager::stop(bool discard)
{
    std::lock_guard<std::mutex> lock(*sinks_mutex_);
    for (auto const& sink : sinks_)
    {
        sink->stop(discard);
    }
}

void Manager::dump(const Log& log, const std::string& channel_name, ContextInfo const& context_info)
{
    dump(log, channel(channel_name), context_info);
}

void Manager::dump(const Log& log, const Channel& channel, ContextInfo const& context_info)
{
    // Copy shared pointer in order to allow update without locking on replace_global_context_info
    // The local copy increments the ref-count and guarantees that the pointed-at context_info will not be deleted
    // while we're working on it, even if the global_context_info_ is replaced with a new context_info pointer
    auto context_info_handle(std::atomic_load(&global_context_info_));
    std::lock_guard<std::mutex> lock(*sinks_mutex_);
    for (auto& sink : sinks_)
    {
        sink->dump(log, channel, context_info, *context_info_handle);
    }
}
void Manager::clear_sinks()
{
    std::lock_guard<std::mutex> lock(*sinks_mutex_);
    sinks_.clear();
}

void Manager::clear_channels()
{
    channels_.clear();
}

const Logger& Manager::global_logger() const
{
    return *global_logger_;
}

Log::LogLevel Manager::get_log_level() const
{
    return default_log_level_;
}

void Manager::set_log_level(Log::LogLevel log_level)
{
    if (log_level == default_log_level_)
    {
        return;
    }
    default_log_level_ = log_level;
    for (auto& channel : channels_)
    {
        channel.second->set_log_level(default_log_level_);
    }
}

bool Manager::has_channel(std::string const& name) const
{
    return channels_.find(name) != channels_.cend();
}

bool Manager::mute_channel(std::string const& name)
{
    auto const itr = channels_.find(name);
    if (itr == channels_.cend())
    {
        return false;
    }
    itr->second->set_log_level(Log::LogLevel::QUIET);
    return true;
}

Manager::GlobalContextInfoTypePtr Manager::global_context_info() const
{
    return std::atomic_load(&global_context_info_);
}

void Manager::replace_global_context_info(ContextInfo context_info)
{
    replace_global_context_info_rvalue(std::move(context_info));
}

void Manager::replace_global_context_info_rvalue(ContextInfo&& context_info)
{
    // First allocate the new ContextInfo, and then atomically replace the pointer held in global_context_info_
    auto new_context_info = std::make_shared<Manager::GlobalContextInfoType>(context_info);
    std::atomic_store(&global_context_info_, std::move(new_context_info));
}

// Calling this method concurrently from multiple threads could result in loss of context info (one of the calls could
// be lost)
void Manager::update_global_context_info(ContextInfo const& new_context_info)
{
    // First make a local copy of the current context info, update it and then replace the global context info
    auto context_info_handle(std::atomic_load(&global_context_info_));
    auto copy_of_current = *context_info_handle;
    copy_of_current.update(new_context_info);
    replace_global_context_info_rvalue(std::move(copy_of_current));
}

void Manager::restart_sinks() noexcept
{
    std::lock_guard<std::mutex> lock(*sinks_mutex_);
    std::for_each(sinks_.cbegin(), sinks_.cend(), [](SinkPtr const& itr) { itr->restart_sink(); });
}

void Manager::child_on_fork() noexcept
{
    sinks_mutex_.fork_reset();
}

} // namespace octo::logger
