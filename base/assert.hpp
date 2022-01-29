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
