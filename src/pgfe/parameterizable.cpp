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

#include "parameterizable.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE bool
Parameterizable::has_parameter(const std::string_view name) const noexcept
{
  return parameter_index(name) < parameter_count();
}

DMITIGR_PGFE_INLINE bool Parameterizable::is_invariant_ok() const noexcept
{
  const bool params_ok = !has_parameters() || (parameter_count() > 0);
  const bool named_params_ok = [this]
  {
    const std::size_t pc{parameter_count()};
    for (std::size_t i{positional_parameter_count()}; i < pc; ++i) {
      if (parameter_index(parameter_name(i)) != i)
        return false;
    }
    return true;
  }();
  return params_ok && named_params_ok;
}

} // namespace dmitigr::pgfe
