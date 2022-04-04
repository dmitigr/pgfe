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

#include "composite.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE int cmp(const Composite& lhs, const Composite& rhs) noexcept
{
  if (const auto lfc = lhs.field_count(), rfc = rhs.field_count(); lfc == rfc) {
    for (std::size_t i{}; i < lfc; ++i) {
      if (lhs.field_name(i) < rhs.field_name(i) || lhs[i] < rhs[i])
        return -1;
      else if (lhs.field_name(i) > rhs.field_name(i) || lhs[i] > rhs[i])
        return 1;
    }
    return 0;
  } else
    return lfc < rfc ? -1 : 1;
}

} // namespace dmitigr::pgfe
