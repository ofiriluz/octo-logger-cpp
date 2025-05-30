#ifndef DUMMY_SINK_HPP_
#define DUMMY_SINK_HPP_

#ifndef _WIN32

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/log-level.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <string>
#include <deque>

namespace octo::logger::unittests
{
class DummySink : public Sink
{
  public:
    struct DumpedLog
    {
        std::string message;
        ContextInfo log_context_info;
        ContextInfo context_info;
        const ContextInfo* context_info_addr;
        ContextInfo global_context_info;
        const ContextInfo* global_context_info_addr;
        std::string channel_name;
        std::string log_level;
    };

  private:
    std::deque<DumpedLog> dumped_logs_;

  public:
    explicit DummySink()
        : Sink(SinkConfig("Dummy", octo::logger::SinkConfig::SinkType::CUSTOM_SINK),
               "tests",
               LineFormat::PLAINTEXT_SHORT)
    {
    }
    ~DummySink() override = default;
    const DumpedLog& last_log() const
    {
        return dumped_logs_.front();
    }
    const std::deque<DumpedLog>& logs() const
    {
        return dumped_logs_;
    }
    void clear_logs()
    {
        dumped_logs_.clear();
    }
    void dump(const Log& log,
              const Channel& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override
    {
        dumped_logs_.push_front(DumpedLog{.message = log.str(),
                                          .log_context_info = log.context_info(),
                                          .context_info = context_info,
                                          .context_info_addr = &context_info,
                                          .global_context_info = global_context_info,
                                          .global_context_info_addr = &global_context_info,
                                          .channel_name = channel.channel_name(),
                                          .log_level = LogLevelUtils::level_to_string(log.log_level())});
    }
};
} // namespace octo::logger::unittests

#endif
#endif
