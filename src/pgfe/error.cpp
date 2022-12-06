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

#include "error.hpp"

#include <cassert>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Error::Error(detail::pq::Result&& result) noexcept
  : Problem{std::move(result)}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Error::is_valid() const noexcept
{
  return static_cast<bool>(pq_result_);
}

DMITIGR_PGFE_INLINE bool Error::is_invariant_ok() const noexcept
{
  const auto sev = severity();
  return (!sev ||
    (sev == Problem_severity::error) ||
    (sev == Problem_severity::fatal) ||
    (sev == Problem_severity::panic)) && Problem::is_invariant_ok();
}

} // namespace dmitigr::pgfe
