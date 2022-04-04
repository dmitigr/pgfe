// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "problem.hpp"
#include "std_system_error.hpp"

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
