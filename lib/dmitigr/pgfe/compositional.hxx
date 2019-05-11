// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITIONAL_HXX
#define DMITIGR_PGFE_COMPOSITIONAL_HXX

#include "dmitigr/pgfe/compositional.hpp"

#include <dmitigr/common/debug.hpp>

#include <functional>
#include <type_traits>

namespace dmitigr::pgfe::detail {

inline bool is_invariant_ok(Compositional& o)
{
  const bool fields_ok = !o.has_fields() || ((o.field_count() > 0));
  const bool field_names_ok = [&]()
  {
    const auto fc = o.field_count();
    using Counter = std::remove_const_t<decltype (fc)>;
    for (Counter i = 0; i < fc; ++i)
      if (o.field_index(o.field_name(i), i) != i)
        return false;
    return true;
  }();

  return fields_ok && field_names_ok;
}

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_COMPOSITIONAL_HXX
