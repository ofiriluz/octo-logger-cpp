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
      global_context_info_(std::make_shared<GlobalContextInfoType>())
{
}

Manager& Manager::instance()
{
    if (!manager_)
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
    // name.data() is not guaranteed to be NUL-terminated, so explicitly creating an std::string key.
    std::string key(name);
    std::lock_guard<std::mutex> lock(channels_mutex_);
    // If the channel exists - return it
    if (auto it = channels_.find(key); it != channels_.end())
    {
        return ChannelView(it->second);
    }
    // Create the channel
    auto [it, _] = channels_.try_emplace(std::move(key), std::make_shared<Channel>(name, default_log_level_));
    return ChannelView(it->second);
}

// NOTE: The returned reference is valid only as long as the caller holds a
// ChannelPtr (e.g. via ChannelView) that keeps the Channel alive. Do not store
// raw references across call sites that may call clear_channels() or
// reset_manager() on another thread.
// TODO <ADB-3405>: Replace this accessor and editable_channel() with a find_channel()
// that returns a ChannelPtr, eliminating the reference-validity problem.
const Channel& Manager::channel(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    auto const it = channels_.find(name);
    if (it != channels_.cend())
    {
        return *it->second;
    }
    throw std::runtime_error("No channel for given name [" + name + "]");
}

Channel& Manager::editable_channel(const std::string& name)
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    auto const it = channels_.find(name);
    if (it != channels_.cend())
    {
        return *it->second;
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
    {
        std::lock_guard<std::mutex> lock(channels_mutex_);
        // Change the default level if requested by config, then propagate to all channels.
        if (config_->has_option(ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL))
        {
            int default_level = 0;
            if (config_->option(ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL, default_level))
            {
                default_log_level_ = static_cast<Log::LogLevel>(default_level);
            }
        }
        for (auto const& [_, channel_ptr] : channels_)
        {
            channel_ptr->set_log_level(default_log_level_);
        }
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
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    for (auto const& sink : sinks_)
    {
        sink->stop(discard);
    }
}

void Manager::dump(const Log& log, const std::string& channel_name, ContextInfo const& context_info)
{
    // channels_ is intentionally NOT locked here: acquiring channels_mutex_ on
    // every log message would serialize all logging behind channel management
    // operations (create_channel, set_log_level, etc.).
    // Known race: a concurrent create_channel() that triggers a rehash will
    // invalidate iterators. In practice channels are created once at startup
    // before any logging begins, so this window is very narrow.
    // See ADB-3406 for a proper lock-free follow-up.
    auto const it = channels_.find(channel_name);
    if (it == channels_.cend())
    {
        return;
    }
    dump(log, *it->second, context_info);
}

void Manager::dump(const Log& log, const Channel& channel, ContextInfo const& context_info)
{
    // The local copy increments the ref-count and guarantees that the pointed-at context_info will not be deleted
    // while we're working on it, even if the global_context_info_ is replaced with a new context_info pointer
    GlobalContextInfoTypePtr context_info_handle;
    {
        std::lock_guard<std::mutex> lock(global_context_info_mutex_);
        context_info_handle = global_context_info_;
    }

    std::lock_guard<std::mutex> lock(sinks_mutex_);
    for (auto& sink : sinks_)
    {
        sink->dump(log, channel, context_info, *context_info_handle);
    }
}
void Manager::clear_sinks()
{
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    sinks_.clear();
}

void Manager::clear_channels()
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    channels_.clear();
}

const Logger& Manager::global_logger() const
{
    return *global_logger_;
}

Log::LogLevel Manager::get_log_level() const
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    return default_log_level_;
}

void Manager::set_log_level(Log::LogLevel log_level)
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    if (log_level == default_log_level_)
    {
        return;
    }
    default_log_level_ = log_level;
    for (auto& [_, channel_ptr] : channels_)
    {
        channel_ptr->set_log_level(default_log_level_);
    }
}

bool Manager::has_channel(std::string const& name) const
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    return channels_.find(name) != channels_.cend();
}

bool Manager::mute_channel(std::string const& name)
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
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
    std::lock_guard<std::mutex> lock(global_context_info_mutex_);
    return global_context_info_;
}

void Manager::replace_global_context_info(ContextInfo context_info)
{
    replace_global_context_info_rvalue(std::move(context_info));
}

void Manager::replace_global_context_info_rvalue(ContextInfo&& context_info)
{
    // First allocate the new ContextInfo, and then lock and replace the pointer held in global_context_info_
    auto new_context_info = std::make_shared<Manager::GlobalContextInfoType>(context_info);
    std::lock_guard<std::mutex> lock(global_context_info_mutex_);
    global_context_info_ = std::move(new_context_info);
}

// Calling this method concurrently from multiple threads could result in loss of context info (one of the calls could
// be lost)
void Manager::update_global_context_info(ContextInfo const& new_context_info)
{
    // First make a local copy of the current context info (under lock), update it and then replace the global context info
    GlobalContextInfoTypePtr context_info_handle;
    {
        std::lock_guard<std::mutex> lock(global_context_info_mutex_);
        context_info_handle = global_context_info_;
    }
    auto copy_of_current = *context_info_handle;
    copy_of_current.update(new_context_info);
    replace_global_context_info_rvalue(std::move(copy_of_current));
}

void Manager::restart_sinks() noexcept
{
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    std::for_each(sinks_.cbegin(), sinks_.cend(), [](SinkPtr const& itr) { itr->restart_sink(); });
}

void Manager::child_on_fork() noexcept
{
    channels_mutex_.fork_reset();
    sinks_mutex_.fork_reset();
    global_context_info_mutex_.fork_reset();
    if (global_context_info_)
    {
        // This is probably unnecessary, since the shared_ptr should only use lock-free atomic operations, but just to
        // be on the safe side...
        // Replace in case there is any internal lock in the shared_ptr which is not fork-safe
        // On fork, there is only one thread, so we can safely replace the pointer without a lock
        global_context_info_ = std::make_shared<GlobalContextInfoType>(*global_context_info_);
    }
}

} // namespace octo::logger
