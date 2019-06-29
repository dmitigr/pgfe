// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/util.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/common/debug.hpp>
#include <dmitigr/common/net.hpp>

#include <cerrno>
#include <cstdint>
#include <limits>
#include <locale>

namespace dmitigr::pgfe::detail {

DMITIGR_PGFE_INLINE std::string unquote_identifier(const std::string& identifier)
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

DMITIGR_PGFE_INLINE int sqlstate_to_int(const char* const code)
{
  DMITIGR_ASSERT(code &&
    (std::isalnum(code[0], std::locale{}) &&
     std::isalnum(code[1], std::locale{}) &&
     std::isalnum(code[2], std::locale{}) &&
     std::isalnum(code[3], std::locale{}) &&
     std::isalnum(code[4], std::locale{}) && code[5] == '\0'));

  const long int result = std::strtol(code, NULL, 36);
  DMITIGR_ASSERT(errno == 0);
  DMITIGR_ASSERT(result >= 0 && result <= std::numeric_limits<int>::max());
  return result;
}

DMITIGR_PGFE_INLINE Socket_readiness poll_sock(const int socket, const Socket_readiness mask, const std::chrono::milliseconds timeout)
{
  using Sock = net::Socket_native;
  using Sock_readiness = net::Socket_readiness;
  using net::poll;
  return static_cast<Socket_readiness>(poll(static_cast<Sock>(socket), static_cast<Sock_readiness>(mask), timeout));
}

} // namespace dmitigr::pgfe::detail

#include "dmitigr/pgfe/implementation_footer.hpp"
