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

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/channel-view.hpp"
#include "octo-logger-cpp/fork-safe-mutex.hpp"
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
  private:
    static std::mutex manager_init_mutex_;
    static std::shared_ptr<Manager> manager_;

    std::unordered_map<std::string, ChannelPtr> channels_;
    std::vector<SinkPtr> sinks_;
    ForkSafeMutex sinks_mutex_;
    ManagerConfigPtr config_;
    Log::LogLevel default_log_level_;
    std::shared_ptr<Logger> global_logger_;

  private:
    Manager();

  public:

    Manager(const Manager& other) = delete;
    Manager& operator=(const Manager& other) = delete;

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
    void dump(const Log& log, const std::string& channel_name, Logger::ContextInfo const& context_info);
    void clear_sinks();
    void clear_channels();
    void restart_sinks() noexcept;
    const Logger& global_logger() const;
    // @brief execute this function on child process after fork before logging anything
    void child_on_fork() noexcept;

    [[nodiscard]] Log::LogLevel get_log_level() const;
    void set_log_level(Log::LogLevel log_level);
};
} // namespace octo::logger

#endif
