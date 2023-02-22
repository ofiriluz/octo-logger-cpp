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

#include "octo-logger-cpp/aws/aws-log-system.hpp"
#include <cstring>

namespace
{
constexpr auto REDACTED_TEXT = "<REDACTED>";

constexpr char const* WHITE_LISTED_HEADERS[] = {
    "Accept:",
    "accept:",
    "content-length:",
    "Content-Length:",
    "content-type:",
    "Content-Type:",
    "date:",
    "Date:",
    "Expect:",
    "expect:",
    "Host:",
    "host:",
    "HTTP/",
    "user-agent:",
    "User-Agent:",
    "x-amz-api-version:",
    "x-amz-date:",
};
} // namespace

namespace octo::logger::aws
{
std::shared_ptr<AwsLogSystem> AwsLogSystem::instance_;

std::string AwsLogSystem::sanitizer_tag_to_string(SanitizerTags tag)
{
    switch (tag)
    {
        case SanitizerTags::ALL_TAG:
            return "ALL";
        case SanitizerTags::AUTHV4_TAG:
            return "AUTHV4";
        case SanitizerTags::AWSCLIENT_TAG:
            return "AWSCLIENT";
        case SanitizerTags::CONFIG_FILE_PROFILE_TAG:
            return "CONFIG_FILE_PROFILE";
        case SanitizerTags::CURL_HTTP_CLIENT_TAG:
            return "CURL_HTTP_CLIENT";
        case SanitizerTags::CURL_TAG:
            return "CURL";
        case SanitizerTags::ERROR_MARSHALLER_TAG:
            return "ERROR_MARSHALLER";
    }
    throw std::runtime_error("Invalid tag");
}

AwsLogSystem::SanitizerTags AwsLogSystem::string_to_sanitizer_tag(const std::string& s)
{
    if (s == "ALL")
    {
        return SanitizerTags::ALL_TAG;
    }
    if (s == "AUTHV4")
    {
        return SanitizerTags::AUTHV4_TAG;
    }
    if (s == "AWSCLIENT")
    {
        return SanitizerTags::AWSCLIENT_TAG;
    }
    if (s == "CONFIG_FILE_PROFILE")
    {
        return SanitizerTags::CONFIG_FILE_PROFILE_TAG;
    }
    if (s == "CURL_HTTP_CLIENT")
    {
        return SanitizerTags::CURL_HTTP_CLIENT_TAG;
    }
    if (s == "CURL")
    {
        return SanitizerTags::CURL_TAG;
    }
    if (s == "ERROR_MARSHALLER")
    {
        return SanitizerTags::ERROR_MARSHALLER_TAG;
    }
    throw std::runtime_error("Invalid tag string");
}

Aws::Utils::Logging::LogLevel AwsLogSystem::octo_log_level_to_aws(Log::LogLevel log_level)
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

Log::LogLevel AwsLogSystem::aws_log_level_to_octo(Aws::Utils::Logging::LogLevel log_level)
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

AwsLogSystem::AwsLogSystem() : logger_("AwsLogSystem")
{
    allowed_sanitizer_tags_ = {SanitizerTags::ALL_TAG};
}

std::shared_ptr<AwsLogSystem> AwsLogSystem::instance()
{
    if (!instance_)
    {
        instance_ = std::shared_ptr<AwsLogSystem>(new AwsLogSystem());
    }
    return instance_;
}

void AwsLogSystem::set_allowed_sanitizers_tags(const std::vector<SanitizerTags>& tags)
{
    allowed_sanitizer_tags_ = tags;
}

const std::vector<AwsLogSystem::SanitizerTags>& AwsLogSystem::allowed_sanitizers_tags() const
{
    return allowed_sanitizer_tags_;
}

Aws::Utils::Logging::LogLevel AwsLogSystem::GetLogLevel() const
{
    return octo_log_level_to_aws(logger_.logger_channel().log_level());
}

void AwsLogSystem::SetLogLevel(Aws::Utils::Logging::LogLevel log_level)
{
    logger_.editable_logger_channel().set_log_level(aws_log_level_to_octo(log_level));
}

void AwsLogSystem::Log(Aws::Utils::Logging::LogLevel log_level, char const* tag, char const* format_str, ...)
{
    logger_.warning("AwsLogSystem") << "Unsupported method [methodAwsLogSystem::Log()] was called! ["
                                    << static_cast<unsigned int>(log_level) << "][" << tag << "]";
}

void AwsLogSystem::LogStream(Aws::Utils::Logging::LogLevel log_level,
                             char const* tag,
                             Aws::OStringStream const& message_stream)
{
    auto message(trim_white_spaces(message_stream));
    if (!process_tag(tag, message))
    {
        message.clear();
        message.append(REDACTED_TEXT);
        logger_.error() << "Failed to process [" << tag << "] message!";
    }
    switch (aws_log_level_to_octo(log_level))
    {
        case Log::LogLevel::TRACE:
            logger_.trace(tag) << message;
            break;
        case Log::LogLevel::DEBUG:
            logger_.debug(tag) << message;
            break;
        case Log::LogLevel::INFO:
            logger_.info(tag) << message;
            break;
        case Log::LogLevel::NOTICE:
            logger_.notice(tag) << message;
            break;
        case Log::LogLevel::WARNING:
            logger_.warning(tag) << message;
            break;
        case Log::LogLevel::ERROR:
            logger_.error(tag) << message;
            break;
        case Log::LogLevel::QUIET:
            break;
    }
}

void AwsLogSystem::Flush()
{
    return;
}

std::string AwsLogSystem::trim_white_spaces(Aws::OStringStream const& message_stream) const
{
    auto const delim_pos = message_stream.str().find_last_not_of(" \n\t");
    if (delim_pos != std::string::npos)
    {
        return message_stream.str().substr(0, delim_pos + 1);
    }
    return "";
}

bool AwsLogSystem::allowed_sanitizer_tag(SanitizerTags tag) const
{
    auto all_tags = std::find(allowed_sanitizer_tags_.begin(), allowed_sanitizer_tags_.end(), SanitizerTags::ALL_TAG);
    return all_tags != std::end(allowed_sanitizer_tags_) ||
           std::find(allowed_sanitizer_tags_.begin(), allowed_sanitizer_tags_.end(), tag) != std::end(allowed_sanitizer_tags_);
}

bool AwsLogSystem::process_tag(char const* tag, std::string& message) const
{
    bool res = false;

    if (message.empty())
    {
        res = true;
    }
    else if (strcmp(tag, "Aws::Config::ConfigFileProfileFSM") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::CONFIG_FILE_PROFILE_TAG))
        {
            res = true;
        }
        else
        {
            res = process_config_file_profile_tag(message);
        }
    }
    else if (strcmp(tag, "AWSAuthV4Signer") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::AUTHV4_TAG))
        {
            res = true;
        }
        else
        {
            res = process_auth_v4_signer_tag(message);
        }
    }
    else if (strcmp(tag, "AWSClient") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::AWSCLIENT_TAG))
        {
            res = true;
        }
        else
        {
            res = process_aws_client_tag(message);
        }
    }
    else if (strcmp(tag, "AWSErrorMarshaller") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::ERROR_MARSHALLER_TAG))
        {
            res = true;
        }
        else
        {
            res = process_error_marshaller_tag(message);
        }
    }
    else if (strcmp(tag, "CURL") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::CURL_TAG))
        {
            res = true;
        }
        else
        {
            res = process_curl_tag(message);
        }
    }
    else if (strcmp(tag, "CurlHttpClient") == 0)
    {
        if (!allowed_sanitizer_tag(SanitizerTags::CURL_HTTP_CLIENT_TAG))
        {
            res = true;
        }
        else
        {
            res = process_curl_http_client_tag(message);
        }
    }
    else if (strcmp(tag, "Aws::Config::AWSConfigFileProfileConfigLoader") == 0 ||
             strcmp(tag, "Aws::Config::AWSProfileConfigLoader") == 0 || strcmp(tag, "Aws_Init_Cleanup") == 0 ||
             strcmp(tag, "ClientConfiguration") == 0 || strcmp(tag, "CurlHandleContainer") == 0 ||
             strcmp(tag, "DefaultAWSCredentialsProviderChain") == 0 || strcmp(tag, "EC2MetadataClient") == 0 ||
             strcmp(tag, "InstanceProfileCredentialsProvider") == 0 || strcmp(tag, "FileSystemUtils") == 0 ||
             strcmp(tag, "ProfileConfigFileAWSCredentialsProvider") == 0 ||
             strcmp(tag, "STSAssumeRoleWithWebIdentityCredentialsProvider") == 0)
    {
        res = true;
    }

    return res;
}

bool AwsLogSystem::process_auth_v4_signer_tag(std::string& message) const
{
    bool redacted = false;

#pragma unroll 7
    for (auto const& itr : {"Calculated sha256",
                            "Canonical Header String:",
                            "Canonical Request String:",
                            "Final computed signing hash:",
                            "Final String to sign:",
                            "Signed Headers value:",
                            "Signing request with:"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            message.resize(strlen(itr));
            redacted = true;
            break;
        }
    }

    if (redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return redacted;
}

bool AwsLogSystem::process_aws_client_tag(std::string& message) const
{
    bool redacted = false;
#pragma unroll 5
    for (auto const& itr : {"Found body, but content-length has not been set",
                            "If the signature check failed.",
                            "Request returned error.",
                            "Request Successfully signed",
                            "Server time is"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            return true;
        }
    }
#pragma unroll 1
    for (auto const& itr : {"HTTP response code:"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            auto const delim_pos = message.find_first_of('\n');
            if (delim_pos != std::string::npos)
            {
                message.resize(delim_pos);
            }
            else
            {
                message.resize(strlen(itr));
            }
            redacted = true;

            break;
        }
    }

    if (redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return redacted;
}

bool AwsLogSystem::process_config_file_profile_tag(std::string& message) const
{
    bool redacted = false;
#pragma unroll 3
    for (auto const& itr : {"found access key", "found profile", "found region"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            message.resize(strlen(itr));
            redacted = true;
            break;
        }
    }

    if (redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return redacted;
}

bool AwsLogSystem::process_curl_http_client_tag(std::string& message) const
{
    bool redacted = false;
    bool white_listed = false;

#pragma unroll 10
    for (auto const& itr : {"Including headers:",
                            "Initializing Curl library with version:",
                            "Making request to",
                            "Obtained connection handle",
                            "Releasing curl handle",
                            "Response body length:",
                            "Response content-length header:",
                            "Returned content type",
                            "Returned http response code"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            return true;
        }
    }

#pragma unroll 1
    for (auto const& itr : {"bytes written to response"})
    {
        if (message.find(itr) != std::string::npos)
        {
            return true;
        }
    }

#pragma unroll sizeof(WHITE_LISTED_HEADERS)
    for (auto const& itr : WHITE_LISTED_HEADERS)
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            white_listed = true;
            break;
        }
    }
    if (!white_listed)
    {
        auto const delim_pos = message.find_first_of(':');
        if (delim_pos != std::string::npos)
        {
            message.resize(delim_pos + 1);
            redacted = true;
        }
    }

    if (!white_listed && redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return white_listed || redacted;
}

bool AwsLogSystem::process_curl_tag(std::string& message) const
{
    bool redacted = false;
    bool white_listed = false;

#pragma unroll 2
    for (auto const& itr : {"(DataIn)", "(DataOut)"})
    {
        if (message.compare(0, strlen(itr), itr) == 0)
        {
            message.resize(strlen(itr));
            redacted = true;
            break;
        }
    }
    if (!redacted)
    {
#pragma unroll 3
        for (auto const& itr : {"(SSLDataIn)", "(SSLDataOut)", "(Text)"})
        {
            if (message.compare(0, strlen(itr), itr) == 0)
            {
                white_listed = true;
                break;
            }
        }
    }

    if (white_listed || redacted)
    {
        // Do nothing but also no need to add this check to later if-else.
    }
    else if (message == "(HeaderIn)")
    {
        white_listed = true;
    }
    else if (message.compare(0, 10, "(HeaderIn)") == 0)
    {
#pragma unroll sizeof(WHITE_LISTED_HEADERS)
        for (auto const& itr : WHITE_LISTED_HEADERS)
        {
            if (message.compare(11, strlen(itr), itr) == 0)
            {
                white_listed = true;
                break;
            }
        }
        if (!white_listed)
        {
            auto const delim_pos = message.find_first_of(':');
            if (delim_pos != std::string::npos)
            {
                message.resize(delim_pos + 1);
            }
            else
            {
                message.resize(10);
            }
            redacted = true;
        }
    }
    else if (message.compare(0, 11, "(HeaderOut)") == 0)
    {
        auto const delim_pos = message.find_first_of('\n');
        if (delim_pos != std::string::npos)
        {
            message.resize(delim_pos);
        }
        else
        {
            message.resize(11);
        }
        redacted = true;
    }

    if (!white_listed && redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return white_listed || redacted;
}

bool AwsLogSystem::process_error_marshaller_tag(std::string& message) const
{
    bool redacted = false;
    if (message.compare(0, 22, "Encountered AWSError '") == 0)
    {
        auto const delim_pos = message.find("':", 21);
        if (delim_pos != std::string::npos)
        {
            message.resize(delim_pos + 1);
        }
        else
        {
            message.resize(20);
        }
        redacted = true;
    }
    else if (message.compare(0, 17, "Error response is") == 0)
    {
        message.resize(17);
        redacted = true;
    }

    if (redacted)
    {
        message.append(" ");
        message.append(REDACTED_TEXT);
    }

    return redacted;
}

} // namespace octo::logger::aws

#endif
