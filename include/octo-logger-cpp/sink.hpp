/**
 * @file sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SINK_HPP_
#define SINK_HPP_

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace octo::logger
{
class Sink
{
  private:
    const SinkConfig config_;

  protected:
    const SinkConfig& config() const;
    std::string formatted_log(Log const& log, Channel const& channel) const;
    [[nodiscard]] virtual std::string formatted_context_info(Log const& log,
                                                             Channel const& channel,
                                                             Logger::ContextInfo const& context_info) const;

  public:
    explicit Sink(const SinkConfig& config);
    virtual ~Sink() = default;

    virtual void dump(const Log& log, const Channel& channel, Logger::ContextInfo const& context_info) = 0;
    const std::string& sink_name() const;
};
typedef std::shared_ptr<Sink> SinkPtr;
} // namespace octo::logger

#endif
