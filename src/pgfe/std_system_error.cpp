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

#include "std_system_error.hpp"
#include "problem.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::string Client_error_category::message(const int ev) const
{
  std::string result{name()};
  result += ' ';
  result += std::to_string(ev);
  if (const char* const literal = to_literal(static_cast<Client_errc>(ev))) {
    result += ' ';
    result += literal;
  }
  return result;
}

DMITIGR_PGFE_INLINE std::string Server_error_category::message(const int ev) const
{
  std::string result{name()};
  result += ' ';
  result += std::to_string(ev);
  result += ' ';
  result += Problem::sqlstate_int_to_string(ev);
  if (const char* const literal = to_literal(static_cast<Server_errc>(ev))) {
    result += ' ';
    result += literal;
  }
  return result;
}

} // namespace dmitigr::pgfe
