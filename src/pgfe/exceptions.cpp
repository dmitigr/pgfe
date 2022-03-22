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

#include "contract.hpp"
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
  : Exception{detail::not_false(error)->condition(), detail::not_false(error)->brief()}
  , error_{std::move(error)}
{}

DMITIGR_PGFE_INLINE const Error& Server_exception::error() const noexcept
{
  return *error_;
}

} // namespace dmitigr::pgfe
