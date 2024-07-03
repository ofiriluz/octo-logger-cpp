/**
 * @file file-sink.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef FILE_SINK_HPP_
#define FILE_SINK_HPP_

#ifndef _WIN32

#include "octo-logger-cpp/channel.hpp"
#include "octo-logger-cpp/log.hpp"
#include "octo-logger-cpp/logger.hpp"
#include "octo-logger-cpp/sink-config.hpp"
#include "octo-logger-cpp/sink.hpp"
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <libgen.h>
#include <string>
#include <string_view>
#include <strings.h>
#include <sys/stat.h>
#include <unordered_map>

namespace octo::logger
{
class FileSink : public Sink
{
  private:
    struct File
    {
        std::ofstream stream;
        uint32_t index;
        File();
    };

  private:
    std::string prefix_folder_name_;
    std::string log_path_;
    std::string combined_channels_prefix_;
    long size_per_file_;
    int max_files_;
    std::map<std::string, std::shared_ptr<File>> current_files_;
    bool separate_channels_to_files_;
    bool separate_logs_by_date_folder_;
    std::string strftime_format_;
    bool disable_file_context_info_;

  private:
    int recursive_folder_creation(const char* dir, mode_t mode);
    void create_log_path();
    void switch_stream(const std::string& channel);

  public:
    explicit FileSink(const SinkConfig& config);
    ~FileSink() override;

    void dump(const Log& log,
              const Channel& channel,
              ContextInfo const& context_info,
              ContextInfo const& global_context_info) override;
};
} // namespace octo::logger

#endif
#endif