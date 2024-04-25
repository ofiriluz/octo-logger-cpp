#ifndef DUMMY_SINK_HPP_
#define DUMMY_SINK_HPP_

#ifndef _WIN32

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <string>
#include <deque>

namespace octo::logger
{
class DummySink : public Sink
{
  public:
    struct DumpedLog
    {
      std::string message;
      Logger::ContextInfo context_info;
      std::string channel_name;
      std::string log_level;
    };

  private:
    std::deque<DumpedLog> dumped_logs_;
  public:
    explicit DummySink(const SinkConfig& config);
    ~DummySink() override = default;
    const DumpedLog& last_log() const;
    const std::deque<DumpedLog>& logs() const;
    void clear_logs();


    void dump(const Log& log, const Channel& channel, Logger::ContextInfo const& context_info) override 
    {
      dumped_logs_.push_front(DumpedLog{
        .message = log.stream()->str(),
        .context_info = context_info,
        .channel_name = channel.channel_name(),
        .log_level = Log::level_to_string(log.log_level())
      })
    }
};
} // namespace octo::logger

#endif
#endif