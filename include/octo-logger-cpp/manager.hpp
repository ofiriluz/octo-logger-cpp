/**
 * @file manager.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include "octo-logger-cpp/channel-view.hpp"
#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/fork-safe-mutex.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/manager-config.hpp"
#include "octo-logger-cpp/sink-factory.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace octo::logger
{
class Manager
{
  public:
    using GlobalContextInfoType = ContextInfo const;
    using GlobalContextInfoTypePtr = std::shared_ptr<GlobalContextInfoType>;

  private:
    static std::mutex manager_init_mutex_;
    static std::shared_ptr<Manager> manager_;

    std::unordered_map<std::string, ChannelPtr> channels_;
    std::vector<SinkPtr> sinks_;
    mutable ForkSafeMutex sinks_mutex_;
    ManagerConfigPtr config_;
    Log::LogLevel default_log_level_;
    std::shared_ptr<Logger> global_logger_;
    // Shared Pointer in order to allow thread safe usage on the 'dump' method with minimal locking
    // The shared pointer is copied under mutex protection to ensure that the pointed-at ContextInfo will
    // not be deleted while being used in the 'dump' method, even if another thread updates the global_context_info_
    /*
     * FIXME: In the future we should switch to std::atomic_shared_ptr (c++20) and then we won't need the mutex here,
     * We are using a mutex for fork-safety, but std::atomic_shared_ptr should inherently be fork-safe, since it should
     * only use lock-free atomic operations (as opposed to std::atomic_load/store on shared_ptr which uses locks internally)
     */
    mutable ForkSafeMutex global_context_info_mutex_;
    GlobalContextInfoTypePtr global_context_info_;

  private:
    explicit Manager();

  public:
    // Non-copyable and non-movable
    Manager(const Manager& other) = delete;
    Manager& operator=(const Manager& other) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    static Manager& instance();
    static void reset_manager();

    virtual ~Manager();

    ChannelView create_channel(std::string_view name);
    Channel& editable_channel(const std::string& name);
    [[nodiscard]] const Channel& channel(const std::string& name) const;
    [[nodiscard]] bool has_channel(std::string const& name) const;
    bool mute_channel(std::string const& name);
    void configure(const ManagerConfigPtr& config, bool clear_old_sinks = true);
    void terminate();
    void stop(bool discard = false);
    void dump(const Log& log, const std::string& channel_name, ContextInfo const& context_info);
    void dump(const Log& log, const Channel& channel, ContextInfo const& context_info);
    void clear_sinks();
    void clear_channels();
    void restart_sinks() noexcept;
    const Logger& global_logger() const;
    GlobalContextInfoTypePtr global_context_info() const;
    void replace_global_context_info(ContextInfo context_info);
    // @brief replace the global context info with an rvalue to avoid extra copying
    // It is not named 'replace_global_context_info' to avoid ambiguity with the lvalue version
    void replace_global_context_info_rvalue(ContextInfo&& context_info);
    void update_global_context_info(ContextInfo const& new_context_info);
    // @brief execute this function on child process after fork before logging anything
    void child_on_fork() noexcept;

    [[nodiscard]] Log::LogLevel get_log_level() const;
    void set_log_level(Log::LogLevel log_level);
};
} // namespace octo::logger

#endif
