octo-logger-cpp
==============

[![Logger Linux Build Pipeline](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/linux.yml/badge.svg)](https://github.com/ofiriluz/octo-logger-cpp/actions/workflows/linux.yml)

Logging library, capabable of logging to different channels from each logger

The logger is configured once globallby via a logger manager.

Afterwards, one can create as many logs as he wants, each log is tagged with the channel name which will be a part of the output

All the log output will be dumped to the sinks that were registered on the global manager

Currently the log supports the following log sinks
- ConsoleSink
- SysLogSink
- FileSink
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
logger.debug() << "HI";
logger.info() << "HI2";
logger.notice() << "HI3";
logger.warning() << "HI4";
logger.error() << "HI5";

logger.debug("ID1") << "HI WITH ID";
logger.info("ID2") << "HI2 WITH ID";
logger.notice("ID3") << "HI3 WITH ID";
logger.warning("ID4") << "HI4 WITH ID";
logger.error("ID5") << "HI5 WITH ID";

logger.log(octo::logger::Log::LogLevel::ERROR, "BLA") << "BLA LOG";

octo::logger::Manager::instance().global_logger().info() << "WTF IS GLOBAL";
```
