/**
 * @file console-json-sink.hpp
 * @author Denis Sheyer (denis.sheyer@gmail.com)
 * @brief
 * @date 12025-01-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef CONSOLE_JSON_SINK_HPP_
#define CONSOLE_JSON_SINK_HPP_

#ifdef OCTO_LOGGER_WITH_JSON_FORMATTING

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/context-info.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <string>

namespace octo::logger
{

/**
 * @brief Sink implementation that dumps logs to the console in JSON format.
 *
 * In the SinkConfig object, the following options are available:
 * - CONSOLE_JSON_HOST: The host name to be included in the JSON log.
 * - CONSOLE_JSON_ORIGIN: The origin name to be included in the JSON log.
 * - CONSOLE_JSON_SERVICE: The service name to be included in the JSON log.
 * - CONSOLE_JSON_INDENT: The indentation level to be used in the JSON log.
 */
class ConsoleJSONSink : public Sink
{
  public:
    static auto constexpr DEFAULT_HOST = "";
    static auto constexpr DEFAULT_ORIGIN = "";
    static auto constexpr DEFAULT_SERVICE = "";
    static int constexpr DEFAULT_INDENT = -1;

  private:
    std::string const host_;
    std::string const service_;
    int const indent_;
    bool const log_thread_id_;

  public:
    explicit ConsoleJSONSink(SinkConfig const& sink_config);
    ~ConsoleJSONSink() override = default;

    void dump(Log const& log,
              Channel const& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override;
};
} // namespace octo::logger

#endif // OCTO_LOGGER_WITH_JSON_FORMATTING

#endif // CONSOLE_JSON_SINK_HPP_
