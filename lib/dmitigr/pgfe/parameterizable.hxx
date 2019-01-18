// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PARAMETERIZABLE_HXX
#define DMITIGR_PGFE_PARAMETERIZABLE_HXX

#include "dmitigr/pgfe/parameterizable.hpp"
#include "dmitigr/internal/debug.hpp"

namespace dmitigr::pgfe::detail {

inline bool is_invariant_ok(const Parameterizable& o)
{
  const bool params_ok = !o.has_parameters() || (o.parameter_count() > 0);
  const bool named_params_ok = [&]()
  {
    const auto pc = o.parameter_count();
    for (auto i = o.positional_parameter_count(); i < pc; ++i) {
      if (o.parameter_index(o.parameter_name(i)) != i)
        return false;
    }
    return true;
  }();

  return params_ok && named_params_ok;
}

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_PARAMETERIZABLE_HXX
