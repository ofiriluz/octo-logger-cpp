#ifndef CATCH2_MATCHERS_HPP_
#define CATCH2_MATCHERS_HPP_

#include "log-mock.hpp"
#include "logger-mock.hpp"
#include "cloudwatch-sink-mock.hpp"
#include <catch2/catch_all.hpp>
#include <fmt/format.h>
#include <string>
#include <memory>

namespace octo::logger::unittests
{

class LogLevelEquals : public Catch::Matchers::MatcherBase<LogMock::LogLevel>
{
  private:
    LogMock::LogLevel log_level_;

  public:
    explicit LogLevelEquals(LogMock::LogLevel log_level) : log_level_(log_level)
    {
    }

    bool match(LogMock::LogLevel const& in) const override
    {
        return log_level_ == in;
    }

    std::string describe() const override
    {
        return fmt::format("equals {}", static_cast<int>(log_level_));
    }
};

class ContextInfoEquals : public Catch::Matchers::MatcherBase<ContextInfo>
{
  private:
    ContextInfo context_;

  public:
    explicit ContextInfoEquals(ContextInfo context)
        : context_(std::move(context))
    {
    }

    bool match(ContextInfo const& in) const override
    {
        return context_ == in;
    }

    std::string describe() const override
    {
        return fmt::format("equals {}", to_string(context_));
    }

    static std::string to_string(ContextInfo const& in)
    {
        std::string context_str("{");
        bool first = true;
        for (auto const& itr : in)
        {
            if (!first)
            {
                context_str += ", ";
            }
            first = false;
            context_str += fmt::format("{{{},{}}}", itr.first, itr.second);
        }
        context_str += "}";
        return std::move(context_str);
    }
};

#ifdef OCTO_LOGGER_WITH_AWS
class LogStreamTypeEquals : public Catch::Matchers::MatcherBase<CloudWatchSinkMock::LogStreamType>
{
  private:
    CloudWatchSinkMock::LogStreamType log_stream_type_;

  public:
    explicit LogStreamTypeEquals(CloudWatchSinkMock::LogStreamType log_stream_type) : log_stream_type_(log_stream_type)
    {
    }

    bool match(CloudWatchSinkMock::LogStreamType const& in) const override
    {
        return log_stream_type_ == in;
    }

    std::string describe() const override
    {
        return fmt::format("equals {}", static_cast<int>(log_stream_type_));
    }
};

class JSONEquals : public Catch::Matchers::MatcherBase<nlohmann::json>
{
  private:
    nlohmann::json const json_;

  public:
    explicit JSONEquals(nlohmann::json json) : json_(std::move(json))
    {
    }

    bool match(nlohmann::json const& in) const override
    {
        return json_ == in;
    }

    std::string describe() const override
    {
        return fmt::format("\nequals\n{}", json_.dump());
    }
};
#endif

} // namespace octo::logger::unittests
#endif // CATCH2_MATCHERS_HPP_
