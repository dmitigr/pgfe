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

#include "contract.hpp"
#include "errctg.hpp"
#include "error.hpp"
#include "exceptions.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Client_exception::Client_exception(const Client_errc errc,
  std::string what)
  : Exception{errc, what.empty() ? to_literal(errc) :
    what.append(" (").append(to_literal(errc)).append(")")}
{}

DMITIGR_PGFE_INLINE Client_exception::Client_exception(const std::string& what)
  : Exception{what}
{}

// =============================================================================

DMITIGR_PGFE_INLINE Server_exception::Server_exception(std::shared_ptr<Error>&& error)
  : Exception{detail::not_false(error)->condition(),
    detail::not_false(error)->brief()}
  , error_{std::move(error)}
{}

DMITIGR_PGFE_INLINE const Error& Server_exception::error() const noexcept
{
  return *error_;
}

DMITIGR_PGFE_INLINE std::shared_ptr<Error> Server_exception::error_ptr() const noexcept
{
  return error_;
}

} // namespace dmitigr::pgfe
