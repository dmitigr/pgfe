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
