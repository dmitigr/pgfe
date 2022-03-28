// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "basics.hpp"
#include "exceptions.hpp"
#include "misc.hpp"
#include "pq.hpp"

#include <locale>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE void set_initialization(const External_library library)
{
  const auto libssl = static_cast<bool>(library & External_library::libssl);
  const auto libcrypto = static_cast<bool>(library & External_library::libcrypto);
  ::PQinitOpenSSL(libssl, libcrypto);
}

DMITIGR_PGFE_INLINE std::string unquote_identifier(const std::string_view identifier)
{
  enum { top, double_quote, adjacent_double_quote } state = top;

  std::string result;
  const std::string_view::size_type sz = identifier.size();
  const std::locale loc;
  for (std::string_view::size_type i = 0; i < sz; ++i) {
    const char c = identifier[i];
    if (state == top) {
      if (c != '"')
        result += std::tolower(c, loc);
      else
        state = double_quote;
    } else if (state == double_quote) {
      if (c != '"')
        result += c;
      else
        state = (i + 1 < sz && identifier[i + 1] == '"') ? adjacent_double_quote : top;
    } else if (state == adjacent_double_quote) {
      result += c;
      state = double_quote;
    }
  }
  return result;
}

DMITIGR_PGFE_INLINE int array_dimension(const std::string_view literal,
  const char delimiter)
{
  if (!literal.data())
    return 0;

  int dimension{};
  const std::locale loc;
  for (const auto c : literal) {
    if (c == '{')
      ++dimension;
    else if (std::isspace(c, loc))
      ; // Skip space.
    else if (!dimension || c == delimiter)
      throw Client_exception{Client_errc::malformed_array_literal};
    else
      break;
  }
  return dimension;
}

} // namespace dmitigr::pgfe
