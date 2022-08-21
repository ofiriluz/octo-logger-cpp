octo-logger-cpp
==============

[![Logger Linux Build Pipeline](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/linux.yml/badge.svg)](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/linux.yml)
[![Logger Windows Build Pipeline](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/windows.yml/badge.svg)](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/windows.yml)
[![Logger Mac Build Pipeline](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/mac.yml/badge.svg)](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/mac.yml)

Logging library, capabable of logging to different channels from each logger

The logger is configured once globallby via a logger manager.

Afterwards, one can create as many logs as he wants, each log is tagged with the channel name which will be a part of the output

All the log output will be dumped to the sinks that were registered on the global manager

Currently the log supports the following log sinks
- ConsoleSink
- SysLogSink
- FileSink
- CloudWatchSink - Only compiled with WITH_AWS flag on and aws-sdk-cpp libraries
- CustomSink - Interface for custom sinks that the user can implement

Note that the logger works in a stream oriented c++ style

Usage
=====

In order to use the logger, one needs to configure the loggers globally, otherwise there are no writers, and nothing will be written anywhere

In order to do so, we will use the PSManager:

```cpp
auto config = std::make_shared<octo::logger::ManagerConfig>();
config->set_option(octo::logger::ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL, static_cast<int>(octo::logger::Log::LogLevel::DEBUG));
octo::logger::SinkConfig file_sink("File", octo::logger::SinkConfig::SinkType::FILE_SINK);
octo::logger::SinkConfig console_sink("Console", octo::logger::SinkConfig::SinkType::CONSOLE_SINK);
file_writer_config.set_option(octo::logger::SinkConfig::SinkOption::FILE_LOG_FILES_PATH, "/tmp/test");
config->add_sink(file_sink);
config->add_writer(console_sink);
octo::logger::Manager::instance().configure(config);
```

This will configure the log level to be defaulted debug for all channels, each channel can also configure its own log level

The sinks will be the filesink and consolesink, each sink has a predefined set of options which can be changed

Once the configuration is done, we set the manager with this config and can now use the logger as follows

```cpp
octo::logger::Logger logger("Test");
logger.debug() << "A";
logger.info() << "B";
logger.notice() << "C";
logger.warning() << "D";
logger.error() << "E";

logger.debug("ID1") << "A WITH ID";
logger.info("ID2") << "B WITH ID";
logger.notice("ID3") << "C WITH ID";
logger.warning("ID4") << "D WITH ID";
logger.error("ID5") << "E WITH ID";

logger.log(octo::logger::Log::LogLevel::ERROR, "ID") << "GENERAL";

octo::logger::Manager::instance().global_logger().info() << "GLOBAL";
```

The logger also supports AWS related loggings using cloudwatch

To use that, you need to consume / compile octo-logger with aws-sdk-cpp

To do so with conan:

```bash
conan install . -o with_aws=True --build=aws-sdk-cpp
```
Note that building of aws-sdk-cpp might not be needed if AWS SDK is compiled with the logs SDK

Once the octo logger is compiled with AWS support, we can use it as follows:

```cpp
// Init AWS
Aws::SDKOptions options;
octo::logger::aws::AwsLogSystem::instance()->SetLogLevel(settings_->logging_settings().aws_log_level());
options.loggingOptions.logLevel = settings_->logging_settings().aws_log_level();
options.loggingOptions.logger_create_fn = octo::logger::aws::AwsLogSystem::instance;
Aws::InitAPI(options);

if (!Aws::Utils::Logging::GetLogSystem())
{
    Aws::Utils::Logging::InitializeAWSLogging(octo::logger::aws::AwsLogSystem::instance());
}

// Add and configure cloudwatch log sink
octo::logger::aws::CloudWatchSink::LogGroupTags log_group_tags{
    {"product", "octo"}
};
std::string task_id = "local-task-"s + uuids::to_string(uuids::uuid_system_generator{}());
if (auto task_md = octo::aws::ECSTaskMetadata::retrieve())
{
    task_id = "task-"s + task_md->task_id();
}
std::string log_group_name = "/octo";
octo::logger::SinkConfig cloudwatch_config("Cloudwatch", octo::logger::SinkConfig::SinkType::CUSTOM_SINK);
octo::logger::SinkPtr cloudwatch_sink = std::make_shared<octo::logger::aws::CloudWatchSink>(
    cloudwatch_config,
    std::move(task_id),
    octo::logger::aws::CloudWatchSink::LogStreamType::BY_EXTRA_ID,
    true,
    log_group_name,
    log_group_tags);
config->add_custom_sink(cloudwatch_sink);
```
