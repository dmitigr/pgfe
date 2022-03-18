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

  /// Non copy-constructible.
  Ready_for_query(const Ready_for_query&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Ready_for_query(Ready_for_query&& rhs) noexcept;

  /// Non copy-assignable.
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

  /// The constructor.
  explicit DMITIGR_PGFE_API Ready_for_query(detail::pq::Result&& pq_result) noexcept;
};

/// Ready_for_query is swappable.
inline void swap(Ready_for_query& lhs, Ready_for_query& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "ready_for_query.cpp"
#endif

#endif  // DMITIGR_PGFE_READY_FOR_QUERY_HPP
