/**
 * @file aws-log-system.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifdef WITH_AWS

#ifndef AWS_LOG_SYSTEM_HPP_
#define AWS_LOG_SYSTEM_HPP_

#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/manager.hpp"
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/LogSystemInterface.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace octo::logger::aws
{

class AwsLogSystem : public Aws::Utils::Logging::LogSystemInterface
{
  public:
    static Aws::Utils::Logging::LogLevel octo_log_level_to_aws(Log::LogLevel log_level)
    {
        switch (log_level)
        {
            case Log::LogLevel::TRACE:
                return Aws::Utils::Logging::LogLevel::Trace;
            case Log::LogLevel::DEBUG:
                return Aws::Utils::Logging::LogLevel::Debug;
            case Log::LogLevel::INFO:
            case Log::LogLevel::NOTICE:
                return Aws::Utils::Logging::LogLevel::Info;
            case Log::LogLevel::WARNING:
                return Aws::Utils::Logging::LogLevel::Warn;
            case Log::LogLevel::ERROR:
                return Aws::Utils::Logging::LogLevel::Error;
            case Log::LogLevel::QUIET:
                return Aws::Utils::Logging::LogLevel::Off;
        }
        throw std::runtime_error("Invalid log level");
    }
    static Log::LogLevel aws_log_level_to_octo(Aws::Utils::Logging::LogLevel log_level)
    {
        switch (log_level)
        {
            case Aws::Utils::Logging::LogLevel::Off:
                return Log::LogLevel::QUIET;
            case Aws::Utils::Logging::LogLevel::Fatal:
            case Aws::Utils::Logging::LogLevel::Error:
                return Log::LogLevel::ERROR;
            case Aws::Utils::Logging::LogLevel::Warn:
                return Log::LogLevel::WARNING;
            case Aws::Utils::Logging::LogLevel::Info:
                return Log::LogLevel::INFO;
            case Aws::Utils::Logging::LogLevel::Debug:
                return Log::LogLevel::DEBUG;
            case Aws::Utils::Logging::LogLevel::Trace:
                return Log::LogLevel::TRACE;
        }
        throw std::runtime_error("Invalid AWS log level");
    }

  private:
    [[nodiscard]] bool process_auth_v4_signer_tag(std::string& message) const;
    [[nodiscard]] bool process_aws_client_tag(std::string& message) const;
    [[nodiscard]] bool process_config_file_profile_tag(std::string& message) const;
    [[nodiscard]] bool process_curl_http_client_tag(std::string& message) const;
    [[nodiscard]] bool process_curl_tag(std::string& message) const;
    [[nodiscard]] bool process_error_marshaller_tag(std::string& message) const;

  protected:
    Logger logger_;
    static std::shared_ptr<AwsLogSystem> instance_;

  protected:
    AwsLogSystem();

    [[nodiscard]] std::string trim_white_spaces(Aws::OStringStream const& message_stream) const;
    [[nodiscard]] bool process_tag(char const* tag, std::string& message) const;

  public:
    AwsLogSystem(AwsLogSystem const&) = delete;
    AwsLogSystem(AwsLogSystem const&&) = delete;
    AwsLogSystem& operator=(AwsLogSystem const&) = delete;
    ~AwsLogSystem() override = default;

    static std::shared_ptr<AwsLogSystem> instance();

    Aws::Utils::Logging::LogLevel GetLogLevel() const override;
    void Log(Aws::Utils::Logging::LogLevel log_level, char const* tag, char const* format_str, ...) override;
    void LogStream(Aws::Utils::Logging::LogLevel log_level,
                   char const* tag,
                   Aws::OStringStream const& message_stream) override;
    void Flush() override;

    void SetLogLevel(Aws::Utils::Logging::LogLevel log_level);
};

} // namespace octo::logger::aws

#endif // AWS_LOG_SYSTEM_HPP_

#endif
