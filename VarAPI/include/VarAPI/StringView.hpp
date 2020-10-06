/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
#ifndef VAR_API_STRING_VIEW_HPP
#define VAR_API_STRING_VIEW_HPP

#include <string_view>

#include "Api.hpp"

namespace var {

class String;

class StringView {
public:
  constexpr static size_t npos = std::string_view::npos;

  StringView() {}
  StringView(const char *value) : m_string_view(value) {}
  StringView(const char *value, size_t length) : m_string_view(value, length) {}
  StringView(const String &value);

  char at(size_t value) const { return m_string_view.at(value); }
  char front() const { return m_string_view.front(); }
  char back() const { return m_string_view.back(); }
  char length() const { return m_string_view.length(); }

  bool is_empty() const { return m_string_view.empty(); }
  StringView &pop_front() {
    m_string_view.remove_prefix(1);
    return *this;
  }

  StringView &pop_back() {
    m_string_view.remove_suffix(1);
    return *this;
  }

  using iterator = typename std::string_view::iterator;
  using const_iterator = typename std::string_view::const_iterator;
  using reverse_iterator = typename std::string_view::reverse_iterator;
  using const_reverse_iterator =
    typename std::string_view::const_reverse_iterator;

  const_iterator begin() const noexcept { return m_string_view.begin(); }
  iterator begin() noexcept { return m_string_view.begin(); }

  const_iterator end() const noexcept { return m_string_view.end(); }
  iterator end() noexcept { return m_string_view.end(); }

  const_iterator cbegin() const noexcept { return m_string_view.cbegin(); }
  const_iterator cend() const noexcept { return m_string_view.cend(); }

  const_reverse_iterator rbegin() const noexcept {
    return m_string_view.rbegin();
  }
  reverse_iterator rbegin() noexcept { return m_string_view.rbegin(); }

  const_reverse_iterator rend() const noexcept { return m_string_view.rend(); }
  reverse_iterator rend() noexcept { return m_string_view.rend(); }

  const_reverse_iterator crbegin() const noexcept {
    return m_string_view.crbegin();
  }
  const_reverse_iterator crend() const noexcept {
    return m_string_view.crend();
  }

  const char *cstring() const { return m_string_view.data(); }

  size_t find(const StringView &a, size_t position = 0) const {
    return m_string_view.find(a.string_view(), position);
  }

  size_t find(char a, size_t position = 0) const {
    return m_string_view.find(a, position);
  }

  size_t find_first_of(const StringView &a, size_t position = 0) const {
    return m_string_view.find_first_of(a.string_view(), position);
  }

  size_t find_first_not_of(const StringView &a, size_t position = 0) const {
    return m_string_view.find_first_not_of(a.string_view(), position);
  }

  size_t reverse_find(const StringView &a, size_t position = npos) const {
    return m_string_view.rfind(a.string_view(), position);
  }

  size_t reverse_find(char a, size_t position = npos) const {
    return m_string_view.rfind(a, position);
  }

  size_t find_last_of(const StringView &a, size_t position = 0) const {
    return m_string_view.find_last_of(a.string_view(), position);
  }

  size_t find_last_not_of(const StringView &a, size_t position = 0) const {
    return m_string_view.find_last_not_of(a.string_view(), position);
  }

  const std::string_view &string_view() const { return m_string_view; }
  std::string_view &string_view() { return m_string_view; }

  bool operator==(const StringView &a) const {
    return a.string_view() == string_view();
  }

  bool operator==(const char *a) const { return StringView(a) == *this; }

private:
  std::string_view m_string_view;
};
} // namespace var

#endif // VAR_API_STRING_VIEW_HPP
