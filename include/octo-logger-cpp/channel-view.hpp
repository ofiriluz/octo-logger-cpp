/**
 * @file channel_view.h
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-14
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CHANNEL_VIEW_HPP_
#define CHANNEL_VIEW_HPP_

#include "octo-logger-cpp/channel.hpp"

namespace octo::logger
{

class ChannelView
{
  private:
    ChannelPtr channel_;

  public:
    explicit ChannelView(ChannelPtr channel) : channel_(std::move(channel))
    {
    }
    ChannelView() = default;
    ~ChannelView() = default;
    ChannelView(const ChannelView&) = default;
    ChannelView(ChannelView&&) = default;
    ChannelView& operator=(const ChannelView&) = default;
    ChannelView& operator=(ChannelView&&) = default;

    [[nodiscard]] const Channel& channel() const
    {
        return *channel_;
    }
    [[nodiscard]] Channel& editable_channel()
    {
        return *channel_;
    }
};

} // namespace octo::logger

#endif
