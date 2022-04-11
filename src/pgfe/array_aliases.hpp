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

#ifndef DMITIGR_PGFE_ARRAY_ALIASES_HPP
#define DMITIGR_PGFE_ARRAY_ALIASES_HPP

#include <optional>
#include <vector>

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Aliases of non-nullable arrays
// -----------------------------------------------------------------------------

/**
 * @ingroup conversions
 *
 * @brief 1-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array1 = C<T, A<T>>;

/**
 * @ingroup conversions
 *
 * @brief 2-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array2 = Array1<Array1<T, C, A>, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 3-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array3 = Array1<Array2<T, C, A>, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 4-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array4 = Array1<Array3<T, C, A>, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 5-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array5 = Array1<Array4<T, C, A>, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 6-dimensional array of non-nullable elements.
 */
template<typename T,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array6 = Array1<Array5<T, C, A>, C, A>;

// -----------------------------------------------------------------------------
// Aliases of nullable arrays
// -----------------------------------------------------------------------------

/**
 * @ingroup conversions
 *
 * @brief 1-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional1 = C<O<T>, A<O<T>>>;

/**
 * @ingroup conversions
 *
 * @brief 2-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional2 = Array_optional1<Array_optional1<T, O, C, A>, O, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 3-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional3 = Array_optional1<Array_optional2<T, O, C, A>, O, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 4-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional4 = Array_optional1<Array_optional3<T, O, C, A>, O, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 5-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional5 = Array_optional1<Array_optional4<T, O, C, A>, O, C, A>;

/**
 * @ingroup conversions
 *
 * @brief 6-dimensional array of nullable elements.
 */
template<typename T,
  template<class> class O = std::optional,
  template<class, class> class C = std::vector,
  template<class> class A = std::allocator>
using Array_optional6 = Array_optional1<Array_optional5<T, O, C, A>, O, C, A>;

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ARRAY_ALIASES_HPP
