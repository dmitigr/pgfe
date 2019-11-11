// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/util.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/util/debug.hpp>
#include <dmitigr/util/net.hpp>
#include <dmitigr/util/string.hpp>

#include <cerrno>
#include <limits>
#include <locale>
#include <type_traits>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE int sqlstate_string_to_int(const std::string& code)
{
  const std::locale l{};
  DMITIGR_REQUIRE(
    code.size() == 5 &&
    std::isalnum(code[0], l) &&
    std::isalnum(code[1], l) &&
    std::isalnum(code[2], l) &&
    std::isalnum(code[3], l) &&
    std::isalnum(code[4], l), std::invalid_argument);

  const long int result = std::strtol(code.c_str(), nullptr, 36);
  DMITIGR_ASSERT(errno == 0);
  DMITIGR_ASSERT(result >= 0 && result <= std::numeric_limits<int>::max());
  return result;
}

DMITIGR_PGFE_INLINE std::string sqlstate_int_to_string(const int code)
{
  DMITIGR_REQUIRE(0 <= code && code <= 60466175, std::invalid_argument);
  return string::to_string(code, 36);
}

namespace detail {

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

inline Socket_readiness poll_sock(const int socket, const Socket_readiness mask,
  const std::optional<std::chrono::milliseconds> timeout)
{
  using Sock = net::Socket_native;
  using Sock_readiness = net::Socket_readiness;
  return static_cast<Socket_readiness>(net::poll(static_cast<Sock>(socket),
      static_cast<Sock_readiness>(mask), timeout ? *timeout : std::chrono::milliseconds{-1}));
}

} // namespace detail
} // namespace dmitigr::pgfe

#include "dmitigr/pgfe/implementation_footer.hpp"
