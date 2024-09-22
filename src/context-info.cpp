/**
 * @file context-info.cpp
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07--1
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "octo-logger-cpp/context-info.hpp"

namespace octo::logger
{

ContextInfo::ContextInfo(ContextInfoInitializerList init) : context_info_(std::move(init))
{
}

[[nodiscard]] bool ContextInfo::operator==(ContextInfo const& other) const
{
    return context_info_ == other.context_info_;
}

void ContextInfo::update(ContextInfoKey key, ContextInfoValue value)
{
    context_info_[key] = std::move(value);
}

void ContextInfo::update(ContextInfo const& other)
{
    for (const auto& [key, value] : other.context_info_)
    {
        context_info_[key] = value;
    }
}

void ContextInfo::erase(ContextInfoKey const& key)
{
    context_info_.erase(key);
}

[[nodiscard]] bool ContextInfo::empty() const
{
    return context_info_.empty();
}

[[nodiscard]] bool ContextInfo::contains(ContextInfoKey const& key) const
{
    return context_info_.find(key) != context_info_.cend();
}

[[nodiscard]] ContextInfo::ContextInfoValue ContextInfo::at(ContextInfoKey const& key) const
{
    return context_info_.at(key);
}

void ContextInfo::clear()
{
    return context_info_.clear();
}

ContextInfo::ContextInfoType::iterator ContextInfo::begin()
{
    return context_info_.begin();
}

ContextInfo::ContextInfoType::const_iterator ContextInfo::begin() const
{
    return context_info_.begin();
}

ContextInfo::ContextInfoType::const_iterator ContextInfo::cbegin() const
{
    return context_info_.cbegin();
}

ContextInfo::ContextInfoType::iterator ContextInfo::end()
{
    return context_info_.end();
}

ContextInfo::ContextInfoType::const_iterator ContextInfo::end() const
{
    return context_info_.end();
}

ContextInfo::ContextInfoType::const_iterator ContextInfo::cend() const
{
    return context_info_.cend();
}

} // namespace octo::logger
