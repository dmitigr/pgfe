// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/sql.hxx"

#include <dmitigr/internal/debug.hpp>

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <locale>

namespace {

/**
 * @internal
 *
 * @returns The next number.
 *
 * @par Thread safety
 * Thread-safe.
 */
std::uint_fast64_t next_number() noexcept
{
  static std::atomic<std::uint_fast64_t> result{0};
  ++result;
  return result;
}

/**
 * @internal
 *
 * @returns The prefix of unique SQL identifier.
 *
 * @par Thread safety
 * Thread-safe.
 */
const std::string& sqlid_prefix()
{
  static const std::string result{"$dmitigr_pgfe$_"};
  return result;
}

} // namespace

std::string dmitigr::pgfe::detail::unique_sqlid()
{
  return sqlid_prefix() + std::to_string(next_number());
}

std::string dmitigr::pgfe::detail::unquote_identifier(const std::string& identifier)
{
  enum { top, double_quote, adjacent_double_quote } state = top;

  std::string result;
  const auto sz = identifier.size();
  using Counter = std::remove_const_t<decltype (sz)>;
  for (Counter i = 0; i < sz; ++i) {
    const char c = identifier[i];
    if (state == top) {
      if (c != '"') {
        result += std::tolower(c, std::locale{});
      } else
        state = double_quote;
    } else if (state == double_quote) {
      if (c != '"')
        result += c;
      else                      // Note: identifier[sz] == 0
        state = (identifier[i + 1] == '"') ? adjacent_double_quote : top;
    } else if (state == adjacent_double_quote) {
      result += c;
      state = double_quote;
    }
  }
  return result;
}

int dmitigr::pgfe::detail::sqlstate_to_int(const char* const code)
{
  DMITIGR_INTERNAL_ASSERT(code &&
    ( std::isalnum(code[0], std::locale{}) &&
      std::isalnum(code[1], std::locale{}) &&
      std::isalnum(code[2], std::locale{}) &&
      std::isalnum(code[3], std::locale{}) &&
      std::isalnum(code[4], std::locale{}) && code[5] == '\0'));

  const long int result_candidate = std::strtol(code, NULL, 36);
  DMITIGR_INTERNAL_ASSERT(errno == 0);
  DMITIGR_INTERNAL_ASSERT(result_candidate >= 0 && result_candidate <= std::numeric_limits<int>::max());
  return result_candidate;
}
