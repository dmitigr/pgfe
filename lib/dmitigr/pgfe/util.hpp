// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_UTIL_HPP
#define DMITIGR_PGFE_UTIL_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <dmitigr/util/debug.hpp>
#include <dmitigr/util/net.hpp>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <limits>
#include <locale>
#include <optional>
#include <string>

namespace dmitigr::pgfe::detail {

/**
 * @returns The case-folded and double-quote processed SQL identifier.
 *
 * @par Thread safety
 * Thread-safe.
 */
inline std::string unquote_identifier(const std::string& identifier)
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

/**
 * @returns `0` if no error, or `EINVAL` otherwise.
 *
 * @par Requires
 * The `code` must not be `nullptr` and must consist of five alphanumeric
 * characters.
 *
 * @par Thread safety
 * Thread-safe.
 */
inline int sqlstate_to_int(const char* const code)
{
  const std::locale l{};
  DMITIGR_ASSERT(code &&
    (std::isalnum(code[0], l) &&
     std::isalnum(code[1], l) &&
     std::isalnum(code[2], l) &&
     std::isalnum(code[3], l) &&
     std::isalnum(code[4], l) && code[5] == '\0'));

  const long int result = std::strtol(code, NULL, 36);
  DMITIGR_ASSERT(errno == 0);
  DMITIGR_ASSERT(result >= 0 && result <= std::numeric_limits<int>::max());
  return result;
}


/**
 * @brief A wrapper around net::poll().
 */
inline Socket_readiness poll_sock(const int socket, const Socket_readiness mask,
  const std::optional<std::chrono::milliseconds> timeout)
{
  using Sock = net::Socket_native;
  using Sock_readiness = net::Socket_readiness;
  return static_cast<Socket_readiness>(net::poll(static_cast<Sock>(socket),
      static_cast<Sock_readiness>(mask), timeout ? *timeout : std::chrono::milliseconds{-1}));
}

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_UTIL_HPP
