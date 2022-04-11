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

#include "compositional.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE bool Compositional::is_invariant_ok() const noexcept
{
  const bool fields_ok = is_empty() || field_count() > 0;
  const bool field_names_ok = [this]
  {
    const std::size_t fc{field_count()};
    for (std::size_t i{}; i < fc; ++i)
      if (field_index(field_name(i), i) != i)
        return false;
    return true;
  }();
  return fields_ok && field_names_ok;
}

} // namespace dmitigr::pgfe
