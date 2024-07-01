/**
 * @file context-info.hpp
 * @author arad yaron (aradyaron98@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CONTEXT_INFO_HPP_
#define CONTEXT_INFO_HPP_

#include <unordered_map>
#include <string>
#include <string_view>
#include <sstream>


namespace octo::logger
{

class ContextInfo
{
public:
    typedef std::unordered_map<std::string_view, std::string> ContextInfoType;
    typedef std::initializer_list<ContextInfoType::value_type> ContextInfoInitializerList;

private:
    ContextInfoType context_info_;

public:
    ContextInfo() = default;

    virtual ~ContextInfo() = default;

    ContextInfo(ContextInfoInitializerList init) : context_info_(init)
    {
    }

    ContextInfo(ContextInfo&&) = default;
    ContextInfo& operator=(ContextInfo&&) = default;
    ContextInfo(ContextInfo const&) = default;
    ContextInfo& operator=(ContextInfo const&) = default;

    ContextInfo operator+(ContextInfo const& other) const
    {
        ContextInfo result = *this;
        for (const auto& [key, value] : other.context_info_)
        {
            result.context_info_[key] = value;
        }
        return result;
    }

    [[nodiscard]] bool operator==(ContextInfo const& other) const
    {
        return context_info_ == other.context_info_;
    }

    template <typename T>
    void update(std::string_view key, const T& value)
    {
        std::ostringstream oss;
        oss << value;
        context_info_[key] = oss.str();
    }

    void update(ContextInfo const& other)
    {
        for (const auto& [key, value] : other.context_info_)
        {
            context_info_[key] = value;
        }
    }

    void erase(std::string_view key)
    {
        context_info_.erase(key);
    }

    [[nodiscard]] bool empty() const
    {
        return context_info_.empty();
    }

    [[nodiscard]] bool contains(const std::string_view& key) const
    {
        return context_info_.find(key) != context_info_.end();
    }

    void clear()
    {
        return context_info_.clear();
    }

    ContextInfoType::iterator begin() 
    {
        return context_info_.begin();
    }

    ContextInfoType::const_iterator begin() const 
    {
        return context_info_.begin();
    }

    ContextInfoType::iterator end() 
    {
        return context_info_.end();
    }

    ContextInfoType::const_iterator end() const 
    {
        return context_info_.end();
    }
};

} // namespace octo::logger

#endif
