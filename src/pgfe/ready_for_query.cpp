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

#include "../base/assert.hpp"
#include "ready_for_query.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE
Ready_for_query::Ready_for_query(detail::pq::Result&& pq_result) noexcept
  : pq_result_{std::move(pq_result)}
{}

DMITIGR_PGFE_INLINE
Ready_for_query::Ready_for_query(Ready_for_query&& rhs) noexcept
  : pq_result_{std::move(rhs.pq_result_)}
{}

DMITIGR_PGFE_INLINE Ready_for_query&
Ready_for_query::operator=(Ready_for_query&& rhs) noexcept
{
  Ready_for_query tmp{std::move(rhs)};
  swap(tmp);
  return *this;
}

DMITIGR_PGFE_INLINE void Ready_for_query::swap(Ready_for_query& rhs) noexcept
{
  using std::swap;
  swap(pq_result_, rhs.pq_result_);
}

DMITIGR_PGFE_INLINE bool Ready_for_query::is_valid() const noexcept
{
  return static_cast<bool>(pq_result_);
}

} // namespace dmitigr::pgfe
