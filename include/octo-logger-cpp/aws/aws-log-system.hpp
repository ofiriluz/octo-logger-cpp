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

#ifdef OCTO_LOGGER_WITH_AWS

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
    enum class SanitizerTags : std::uint8_t
    {
        ALL_TAG,
        AUTHV4_TAG,
        AWSCLIENT_TAG,
        CONFIG_FILE_PROFILE_TAG,
        CURL_HTTP_CLIENT_TAG,
        CURL_TAG,
        ERROR_MARSHALLER_TAG
    };

  public:
    static std::string sanitizer_tag_to_string(SanitizerTags tag);
    static SanitizerTags string_to_sanitizer_tag(const std::string& s);
    static Aws::Utils::Logging::LogLevel octo_log_level_to_aws(Log::LogLevel log_level);
    static Log::LogLevel aws_log_level_to_octo(Aws::Utils::Logging::LogLevel log_level);

  private:
    [[nodiscard]] bool process_auth_v4_signer_tag(std::string& message) const;
    [[nodiscard]] bool process_aws_client_tag(std::string& message) const;
    [[nodiscard]] bool process_config_file_profile_tag(std::string& message) const;
    [[nodiscard]] bool process_curl_http_client_tag(std::string& message) const;
    [[nodiscard]] bool process_curl_tag(std::string& message) const;
    [[nodiscard]] bool process_error_marshaller_tag(std::string& message) const;

  protected:
    Logger logger_;
    std::vector<SanitizerTags> allowed_sanitizer_tags_;
    static std::shared_ptr<AwsLogSystem> instance_;

  protected:
    AwsLogSystem();

    [[nodiscard]] std::string trim_white_spaces(Aws::OStringStream const& message_stream) const;
    [[nodiscard]] bool process_tag(char const* tag, std::string& message) const;
    [[nodiscard]] bool allowed_sanitizer_tag(SanitizerTags tag) const;

  public:
    AwsLogSystem(AwsLogSystem const&) = delete;
    AwsLogSystem(AwsLogSystem const&&) = delete;
    AwsLogSystem& operator=(AwsLogSystem const&) = delete;
    ~AwsLogSystem() override = default;

    static std::shared_ptr<AwsLogSystem> instance();

    void set_allowed_sanitizers_tags(const std::vector<SanitizerTags>& tags);
    const std::vector<SanitizerTags>& allowed_sanitizers_tags() const;

    Aws::Utils::Logging::LogLevel GetLogLevel() const override;
    void Log(Aws::Utils::Logging::LogLevel log_level, char const* tag, char const* format_str, ...) override;
    void vaLog(Aws::Utils::Logging::LogLevel log_level, const char* tag, const char* format_str, va_list args) override;
    void LogStream(Aws::Utils::Logging::LogLevel log_level,
                   char const* tag,
                   Aws::OStringStream const& message_stream) override;
    void Flush() override;

    void SetLogLevel(Aws::Utils::Logging::LogLevel log_level);
};

} // namespace octo::logger::aws

#endif // AWS_LOG_SYSTEM_HPP_

#endif
