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

#ifndef DMITIGR_PGFE_ERROR_HPP
#define DMITIGR_PGFE_ERROR_HPP

#include "dll.hpp"
#include "problem.hpp"
#include "response.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A error message from a PostgreSQL server.
 */
class Error final : public Response, public Problem {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Error() = default;

  /// The constructor.
  explicit DMITIGR_PGFE_API Error(detail::pq::Result&& result) noexcept;

  /// @see Message::is_valid().
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

private:
  bool is_invariant_ok() const noexcept override;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "error.cpp"
#endif

#endif  // DMITIGR_PGFE_ERROR_HPP
