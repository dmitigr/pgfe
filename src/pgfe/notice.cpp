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

#include "notice.hpp"

#include <cassert>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Notice::~Notice()
{
  pq_result_.release(); // freed in libpq/fe-protocol3.c:pqGetErrorNotice3()
}

DMITIGR_PGFE_INLINE Notice::Notice(const PGresult* const result) noexcept
  : Problem{detail::pq::Result{const_cast<PGresult*>(result)}}
{
  /*
   * In fact result is not const. So it's okay to const_cast.
   * (Allocated in libpq/fe-protocol3.c:pqGetErrorNotice3().)
   */
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Notice::is_valid() const noexcept
{
  return static_cast<bool>(pq_result_);
}

DMITIGR_PGFE_INLINE bool Notice::is_invariant_ok() const noexcept
{
  const auto sev = severity();
  return ((static_cast<int>(sev) == -1) ||
    (sev == Problem_severity::log) ||
    (sev == Problem_severity::info) ||
    (sev == Problem_severity::debug) ||
    (sev == Problem_severity::notice) ||
    (sev == Problem_severity::warning)) && Problem::is_invariant_ok();
}

} // namespace dmitigr::pgfe
