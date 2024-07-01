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
    typedef std::string_view ContextInfoKey;
    typedef std::string ContextInfoValue;
    typedef std::unordered_map<ContextInfoKey, ContextInfoValue> ContextInfoType;
    typedef std::initializer_list<ContextInfoType::value_type> ContextInfoInitializerList;

private:
    ContextInfoType context_info_;

public:
    ContextInfo() = default;

    virtual ~ContextInfo() = default;

    ContextInfo(ContextInfoInitializerList init);

    ContextInfo(ContextInfo&&) = default;
    ContextInfo& operator=(ContextInfo&&) = default;
    ContextInfo(ContextInfo const&) = default;
    ContextInfo& operator=(ContextInfo const&) = default;
    [[nodiscard]] ContextInfo operator+(ContextInfo const& other) const;
    ContextInfo& operator+=(ContextInfo const& other);
    [[nodiscard]] bool operator==(ContextInfo const& other) const;
    void update(ContextInfoKey key, ContextInfoValue value);
    void update(ContextInfo const& other);
    void erase(ContextInfoKey key);
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool contains(ContextInfoKey key) const;
    void clear();
    [[nodiscard]] ContextInfoType::iterator begin();
    [[nodiscard]] ContextInfoType::iterator end();
    [[nodiscard]] ContextInfoType::const_iterator begin() const;
    [[nodiscard]] ContextInfoType::const_iterator end() const;
    [[nodiscard]] ContextInfoType::const_iterator cbegin() const;
    [[nodiscard]] ContextInfoType::const_iterator cend() const;
};

} // namespace octo::logger

#endif
