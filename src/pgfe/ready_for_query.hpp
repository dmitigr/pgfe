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

#ifndef DMITIGR_PGFE_READY_FOR_QUERY_HPP
#define DMITIGR_PGFE_READY_FOR_QUERY_HPP

#include "dll.hpp"
#include "pq.hpp"
#include "response.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An indicator of the server readiness for new requests.
 *
 * @remarks This response can be only received in pipeline mode.
 *
 * @see Connection::set_pipeline_enabled().
 */
class Ready_for_query final : public Response {
public:
  /// Constructs invalid instance.
  Ready_for_query() noexcept = default;

  /// Not copy-constructible.
  Ready_for_query(const Ready_for_query&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Ready_for_query(Ready_for_query&& rhs) noexcept;

  /// Not copy-assignable.
  Ready_for_query& operator=(const Ready_for_query&) = delete;

  /// Move-assignable.
  DMITIGR_PGFE_API Ready_for_query& operator=(Ready_for_query&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Ready_for_query& rhs) noexcept;

  /// @returns `true` if this instance is correctly initialized.
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

private:
  friend Connection;

  detail::pq::Result pq_result_;

  explicit Ready_for_query(detail::pq::Result&& pq_result) noexcept;
};

/**
 * @ingroup main
 *
 * @brief Ready_for_query is swappable.
 */
inline void swap(Ready_for_query& lhs, Ready_for_query& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "ready_for_query.cpp"
#endif

#endif  // DMITIGR_PGFE_READY_FOR_QUERY_HPP
