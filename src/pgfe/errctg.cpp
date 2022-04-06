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

#include "errctg.hpp"
#include "problem.hpp"

#include <cstring> // std::strlen

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::string Client_error_category::message(const int ev) const
{
  const char* const desc{to_literal_anyway(static_cast<Client_errc>(ev))};
  constexpr const char* const sep{": "};
  std::string result;
  result.reserve(std::strlen(name()) + std::strlen(sep) + std::strlen(desc));
  return result.append(name()).append(sep).append(desc);
}

DMITIGR_PGFE_INLINE std::string Server_error_category::message(const int ev) const
{
  const char* const desc{to_literal_anyway(static_cast<Client_errc>(ev))};
  constexpr const char* const sep{": "};
  const auto sqlstate = Problem::sqlstate_int_to_string(ev);
  std::string result;
  result.reserve(std::strlen(name()) + std::strlen(sep) + std::strlen(desc)
    + 2 + sqlstate.size() + 1);
  return result.append(name()).append(sep).append(desc)
    .append(" (").append(sqlstate).append(")");
}

} // namespace dmitigr::pgfe
