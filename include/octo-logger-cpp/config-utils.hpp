/**
 * @file config-utils.hpp
 * @author Denis Sheyer (denis.sheyer@gmail.com)
 * @brief
 * @date 2025-01-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef CONFIG_UTILS_HPP_
#define CONFIG_UTILS_HPP_

#include <sstream>
#include <string>

namespace octo::logger
{
class ConfigUtils
{
  public:
    ConfigUtils() = delete;

    template <typename T>
    static T convert_to(std::string const& input)
    {
        std::stringstream convertor;
        convertor << input;
        T out;
        convertor >> out;
        return out;
    }

    template <typename T>
    static std::string convert_from(T const& input)
    {
        std::stringstream convertor;
        convertor << input;
        return convertor.str();
    }
};
} // namespace octo::logger

#endif // CONFIG_UTILS_HPP_
