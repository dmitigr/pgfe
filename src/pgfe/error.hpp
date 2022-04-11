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
  Error() noexcept = default;

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
