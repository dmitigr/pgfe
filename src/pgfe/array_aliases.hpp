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

#ifndef DMITIGR_PGFE_ARRAY_ALIASES_HPP
#define DMITIGR_PGFE_ARRAY_ALIASES_HPP

#include <optional>
#include <vector>

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Aliases of non-nullable arrays
// ---------------------------------------------------------------------------

/// 1-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array = C<T, A<T>>;

/// 1-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array1 = Array<T, C, A>;

/// 2-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array2 = Array<Array1<T, C, A>, C, A>;

/// 3-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array3 = Array<Array2<T, C, A>, C, A>;

/// 4-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array4 = Array<Array3<T, C, A>, C, A>;

/// 5-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array5 = Array<Array4<T, C, A>, C, A>;

/// 6-dimensional array of non-nullable elements.
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array6 = Array<Array5<T, C, A>, C, A>;

// -----------------------------------------------------------------------------
// Aliases of nullable arrays
// ---------------------------------------------------------------------------

/// 1-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional = C<O<T>, A<O<T>>>;

/// 1-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional1 = Array_optional<T, O, C, A>;

/// 2-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional2 = Array_optional<Array_optional1<T, O, C, A>, O, C, A>;

/// 3-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional3 = Array_optional<Array_optional2<T, O, C, A>, O, C, A>;

/// 4-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional4 = Array_optional<Array_optional3<T, O, C, A>, O, C, A>;

/// 5-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional5 = Array_optional<Array_optional4<T, O, C, A>, O, C, A>;

/// 6-dimensional array of nullable elements.
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional6 = Array_optional<Array_optional5<T, O, C, A>, O, C, A>;

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ARRAY_ALIASES_HPP
