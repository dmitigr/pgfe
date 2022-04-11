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
  if (this != &rhs) {
    Ready_for_query tmp{std::move(rhs)};
    swap(tmp);
  }
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
