/**
 * @file file-sink.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _WIN32

#include "octo-logger-cpp/sinks/file-sink.hpp"

#include "octo-logger-cpp/compat.hpp"
#include <cstring>
#include <ctime>
#include <ctime>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
std::size_t constexpr TIME_FORMAT_SIZE = 1024;
}

namespace octo::logger
{
FileSink::File::File() : index(0)
{
}

void FileSink::create_log_path()
{
    if (log_path_.data()[log_path_.size() - 1] != '/')
    {
        log_path_ += '/';
    }
    log_path_ += prefix_folder_name_;
    if (!prefix_folder_name_.empty())
    {
        log_path_ += "_";
    }
    if (separate_logs_by_date_folder_)
    {
        char dtf[TIME_FORMAT_SIZE];
        std::time_t const time_v = std::time(nullptr);
        struct tm timeinfo = {};
        std::strftime(dtf, sizeof(dtf), "%d-%m-%Y", compat::localtime(&time_v, &timeinfo, safe_localtime_utc_));
        log_path_ += std::string(dtf);
    }
}

int FileSink::recursive_folder_creation(const char* dir, mode_t mode)
{
    struct stat sb;

    if (!dir)
    {
        errno = EINVAL;
        return 1;
    }

    if (!stat(dir, &sb))
    {
        return 0;
    }

    std::string const temp(dir);
    recursive_folder_creation(dirname(const_cast<char*>(temp.c_str())), mode);

    return mkdir(dir, mode);
}

void FileSink::switch_stream(const std::string& channel)
{
    std::shared_ptr<File> file;
    std::stringstream ss;
    if (separate_channels_to_files_)
    {
        ss << channel;
    }
    else
    {
        ss << combined_channels_prefix_;
    }
    if (!strftime_format_.empty())
    {
        char dtf[TIME_FORMAT_SIZE] = {};
        std::time_t const time_v = std::time(nullptr);
        struct tm timeinfo = {};
        std::strftime(dtf, sizeof(dtf), strftime_format_.c_str(), compat::localtime(&time_v, &timeinfo, safe_localtime_utc_));
        ss << "_" << dtf;
    }
    // If Channel file exists, close it and move to the index for this channel
    if (current_files_.find(channel) != current_files_.end())
    {
        file = current_files_[channel];
        if (file->stream.is_open())
        {
            file->stream.close();
        }
        file->index++;
    }
    // Create new file for the channel
    else
    {
        current_files_[channel] = std::make_shared<File>();
        file = current_files_[channel];
        // List the dir to see if there were existing files for that channel already
        // Update the index accordingly
        std::unique_ptr<DIR, int (*)(DIR*)> dir{opendir(log_path_.c_str()), closedir};
        if (dir != nullptr)
        {
            struct dirent* dir_entry;
            int last_index = 0;
            while ((dir_entry = readdir(dir.get())) != nullptr)
            {
                // Check if the dir entry is a file
                std::string name = dir_entry->d_name;
                // Check if the file is in the format we are looking for
                if (name.find(ss.str()) != std::string::npos)
                {
                    // Extract the index portion of the file
                    // Remove the prefix and the post fix
                    name = name.substr(ss.str().size(), name.size() - ss.str().size() - 4); // remove the .log
                    if (!name.empty())
                    {
                        name = name.substr(1); // Remove the first dot before the file index
                        // Try to cast it to int
                        try
                        {
                            int index = std::stoi(name);
                            if (index > last_index)
                            {
                                last_index = index;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            // Ignore the error
                            // In worst case, we will be appending to an existing file
                        }
                    }
                    else
                    {
                        // We found at least one file so far, so index needs to be updated to 1
                        last_index = 1;
                    }
                }
            }
            // If we found an index file, we just create a new one with the next index
            if (last_index > 0)
            {
                file->index = last_index + 1;
            }
        }
    }

    if (file->index == 0)
    {
        ss << ".log";
    }
    else
    {
        ss << "." << file->index << ".log";
    }
    file->stream.open(std::string(log_path_) + "/" + ss.str().data(), std::ofstream::out | std::ofstream::app);
}

FileSink::FileSink(const SinkConfig& config)
    : Sink(config, "", extract_format_with_default(config, LineFormat::PLAINTEXT_LONG))
{
    combined_channels_prefix_ = config.option_default(SinkConfig::SinkOption::FILE_COMBINED_CHANNEL_PREFIX, "ALL");
    prefix_folder_name_ = config.option_default(SinkConfig::SinkOption::FILE_LOG_FOLDER_PREFIX, "");
    log_path_ = config.option_default(SinkConfig::SinkOption::FILE_LOG_FILES_PATH, "./");
    size_per_file_ = config.option_default(SinkConfig::SinkOption::FILE_SIZE_PER_LOG_FILE, 1024 * 1024);
    max_files_ = config.option_default<int>(SinkConfig::SinkOption::FILE_MAX_LOG_FILES, -1);
    separate_channels_to_files_ = config.option_default(SinkConfig::SinkOption::FILE_SEPARATE_CHANNEL_FILES, false);
    separate_logs_by_date_folder_ = !config.has_option(SinkConfig::SinkOption::FILE_LOG_FOLDER_NO_SEPARATE_BY_DATE);
    strftime_format_ = "";
    if (!config.has_option(SinkConfig::SinkOption::FILE_NO_DATE_ON_NAME))
    {
        strftime_format_ += "%d-%m-%Y";
    }
    if (!config.has_option(SinkConfig::SinkOption::FILE_NO_TIME_ON_NAME))
    {
        if (!strftime_format_.empty())
        {
            strftime_format_ += "_";
        }
        strftime_format_ += "%H-%M-%S";
    }

    create_log_path();
    recursive_folder_creation(log_path_.c_str(), S_IRWXU | S_IRWXG);

    if (!separate_channels_to_files_)
    {
        switch_stream(combined_channels_prefix_);
    }

    disable_file_context_info_ = Sink::config().option_default(SinkConfig::SinkOption::FILE_DISABLE_CONTEXT_INFO, true);
}

FileSink::~FileSink()
{
    for (auto&& file : current_files_)
    {
        file.second->stream.close();
    }
}

void FileSink::dump(const Log& log,
                    const Channel& channel,
                    ContextInfo const& context_info,
                    ContextInfo const& global_context_info)
{
    std::string channel_name;

    if (!separate_channels_to_files_)
    {
        channel_name = combined_channels_prefix_;
    }
    else
    {
        channel_name = channel.channel_name();
        if (current_files_.find(channel_name) == current_files_.end())
        {
            current_files_[channel_name] = std::make_shared<File>();
            switch_stream(channel_name);
        }
    }

    std::shared_ptr<File> const file = current_files_[channel_name];
    if (max_files_ != -1 && file->index > static_cast<std::uint32_t>(max_files_))
    {
        return;
    }

    if (file->stream.tellp() > size_per_file_)
    {
        switch_stream(channel_name);
    }

    if (!log.has_stream())
    {
        return;
    }

    file->stream << formatted_log(log, channel, context_info, global_context_info, disable_file_context_info_);
    file->stream << std::endl;
}
} // namespace octo::logger

#endif
