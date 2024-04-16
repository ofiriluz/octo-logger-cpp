/**
 * @file channel.h
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include "log.hpp"
#include <memory>
#include <string_view>
#include <thread>
#include <memory>

namespace octo::logger
{
class Channel
{
  private:
    std::string channel_name_;
    Log::LogLevel channel_level_;

  public:
    Channel(std::string_view channel_name, const Log::LogLevel& channel_level);
    virtual ~Channel() = default;

    Log::LogLevel log_level() const;
    void set_log_level(const Log::LogLevel& channel_level);
    const std::string& channel_name() const;
    friend class Manager;
};
typedef std::shared_ptr<Channel> ChannelPtr;

} // namespace octo::logger

#endif
