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

#ifndef DMITIGR_BASE_ASSERT_HPP
#define DMITIGR_BASE_ASSERT_HPP

#include <exception> // std::terminate
#ifndef DMITIGR_ASSERT
#include <iostream>
#endif

namespace dmitigr {

/// The debug mode indicator.
#ifndef NDEBUG
constexpr bool is_debug{true};
#else
constexpr bool is_debug{false};
#endif

} // namespace dmitigr

#ifndef DMITIGR_ASSERT
/**
 * @brief Checks the assertion `a`.
 *
 * @details Always active regardless of `is_debug` (or `NDEBUG`).
 *
 * @par Effects Terminates the process if `!a`.
 */
#define DMITIGR_ASSERT(a) do {                                          \
    if (!(a)) {                                                         \
      std::cerr<<"assertion ("<<#a<<") failed at "<<__FILE__<<":"<<__LINE__<<"\n"; \
      std::terminate();                                                 \
    }                                                                   \
  } while (false)
#endif

#endif  // DMITIGR_BASE_ASSERT_HPP
