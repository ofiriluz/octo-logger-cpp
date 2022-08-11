#ifndef CATCH2_MATCHERS_HPP_
#define CATCH2_MATCHERS_HPP_

#include "log-mock.hpp"
#include "logger-mock.hpp"
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

class ContextInfoEquals : public Catch::Matchers::MatcherBase<LoggerMock::ContextInfo>
{
  private:
    LoggerMock::ContextInfo context_;
    decltype(context_)::size_type context_elements_;

  public:
    explicit ContextInfoEquals(LoggerMock::ContextInfo context)
        : context_(std::move(context)), context_elements_(context_.size())
    {
    }

    bool match(LoggerMock::ContextInfo const& in) const override
    {
        if (context_.empty())
        {
            return in.empty();
        }
        else if (in.empty())
        {
            return false;
        }

        int in_elements = 0;
        for (auto const& in_itr : in)
        {
            ++in_elements;
            auto const& context_itr = context_.find(in_itr.first);
            if (context_itr == context_.cend())
            {
                return false;
            }
            if (context_itr->second != in_itr.second)
            {
                return false;
            }
        }
        return context_elements_ == in_elements;
    }

    std::string describe() const override
    {
        return fmt::format("equals {}", to_string(context_));
    }

    static std::string to_string(LoggerMock::ContextInfo const& in)
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

} // namespace octo::logger::unittests
#endif // CATCH2_MATCHERS_HPP_
