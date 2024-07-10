/**
 * @file basic_logger.cpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "octo-logger-cpp/manager.hpp"
#include "octo-logger-cpp/logger.hpp"

int main(int argc, char** argv)
{
    auto config = std::make_shared<octo::logger::ManagerConfig>();
    config->set_option(octo::logger::ManagerConfig::LoggerOption::DEFAULT_CHANNEL_LEVEL,
                       static_cast<int>(octo::logger::Log::LogLevel::TRACE));
    octo::logger::SinkConfig console_sink("Console", octo::logger::SinkConfig::SinkType::CONSOLE_SINK);
    config->add_sink(console_sink);
    octo::logger::Manager::instance().configure(config);

    octo::logger::Logger logger("Test");
    logger.trace() << "HI1";
    logger.debug() << "HI2";
    logger.info() << "HI3";
    logger.notice() << "HI4";
    logger.warning() << "HI5";
    logger.error() << "HI6";

    logger.trace("ID1") << "HI1 WITH ID";
    logger.debug("ID2") << "HI2 WITH ID";
    logger.info("ID3") << "HI3 WITH ID";
    logger.notice("ID4") << "HI4 WITH ID";
    logger.warning("ID5") << "HI5 WITH ID";
    logger.error("ID6") << "HI6 WITH ID";
    
    auto context_info = octo::logger::Manager::instance().global_context_info();
    context_info.update("global_context", "I'm Global Context");
    octo::logger::Manager::instance().set_global_context_info(std::move(context_info));

    logger.add_context_key("logger_context", "I'm Logger Context");
    logger.trace("ID1", {{"log_context", "I'm Log Context"}}) << "HI1 WITH ID";
    logger.debug("ID2",{{"log_context", "I'm Log Context"}}) << "HI2 WITH ID";
    logger.info("ID3",{{"log_context", "I'm Log Context"}}) << "HI3 WITH ID";
    logger.notice("ID4",{{"log_context", "I'm Log Context"}}) << "HI4 WITH ID";
    logger.warning("ID5", {{"log_context", "I'm Log Context"}}) << "HI5 WITH ID";
    logger.error("ID6", {{"log_context", "I'm Log Context"}}) << "HI6 WITH ID";

    logger.log(octo::logger::Log::LogLevel::ERROR, "BLA") << "BLA LOG";

    octo::logger::Manager::instance().global_logger().info() << "WTF IS GLOBAL";

    octo::logger::Manager::instance().terminate();
}