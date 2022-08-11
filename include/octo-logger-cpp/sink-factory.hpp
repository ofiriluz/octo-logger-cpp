/**
 * @file sink-factory.hpp
 * @author ofir iluz (iluzofir@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SINK_FACTORY_HPP_
#define SINK_FACTORY_HPP_

#include "sink.hpp"

namespace octo::logger
{
class SinkFactory
{
  private:
    SinkFactory() = default;

  public:
    SinkFactory(const SinkFactory& other) = delete;
    SinkFactory operator=(const SinkFactory& other) = delete;

    static SinkFactory& instance();
    virtual ~SinkFactory() = default;
    SinkPtr create_sink(const SinkConfig& sink_config);
};
} // namespace octo::logger

#endif