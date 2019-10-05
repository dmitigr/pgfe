// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_DEBUG_HPP
#define DMITIGR_UTIL_DEBUG_HPP

#include "dmitigr/util/macros.hpp"

#include <cstdio>
#include <stdexcept>

namespace dmitigr {

/**
 * @brief The debug mode indicator.
 */
#ifdef NDEBUG
constexpr bool is_debug_enabled = false;
#else
constexpr bool is_debug_enabled = true;
#endif

} // namespace dmitigr

#define DMITIGR_DOUT__(...) {                                           \
    std::fprintf(stderr, "Debug output from " __FILE__ ":" DMITIGR_XSTRINGIZED(__LINE__) ": " __VA_ARGS__); \
  }

#define DMITIGR_ASSERT__(a, t) {                                        \
    if (!(a)) {                                                         \
      DMITIGR_DOUT__("assertion (%s) failed\n", #a)                     \
      if constexpr (t)                                                  \
        throw std::logic_error{"assertion (" #a ") failed at " __FILE__ ":" DMITIGR_XSTRINGIZED(__LINE__)}; \
    }                                                                   \
  }

/**
 * @brief Prints the debug output even when `(is_debug_enabled == false)`.
 */
#define DMITIGR_DOUT_ALWAYS(...) DMITIGR_DOUT__(__VA_ARGS__)

/**
 * @brief Checks the assertion even when `(is_debug_enabled == false)`.
 *
 * @throws An instance of `std::logic_error` if assertion failure.
 */
#define DMITIGR_ASSERT_ALWAYS(a) DMITIGR_ASSERT__(a, true)

/**
 * @brief Checks the assertion even when `(is_debug_enabled == false)`.
 */
#define DMITIGR_ASSERT_NOTHROW_ALWAYS(a) DMITIGR_ASSERT__(a, false)

#define DMITIGR_IF_DEBUG__(code) if constexpr (dmitigr::is_debug_enabled) { code }

/**
 * @brief Prints the debug output only when `(is_debug_enabled == true)`.
 */
#define DMITIGR_DOUT(...) { DMITIGR_IF_DEBUG__(DMITIGR_DOUT_ALWAYS(__VA_ARGS__)) }

/**
 * @brief Checks the assertion only when `(is_debug_enabled == true)`.
 *
 * @throws An instance of `std::logic_error` if assertion failure.
 */
#define DMITIGR_ASSERT(a) { DMITIGR_IF_DEBUG__(DMITIGR_ASSERT_ALWAYS(a)) }

/**
 * @brief Checks the assertion only when `(is_debug_enabled == true)`.
 */
#define DMITIGR_ASSERT_NOTHROW(a) { DMITIGR_IF_DEBUG__(DMITIGR_ASSERT_NOTHROW_ALWAYS(a)) }

/**
 * @throws An instance of type `Exc` with a
 * message about an API `req` (requirement) violation.
 */
#define DMITIGR_THROW_REQUIREMENT_VIOLATED(req, Exc) {                  \
    throw Exc{"API requirement (" #req ") violated at " __FILE__ ":" DMITIGR_XSTRINGIZED(__LINE__)}; \
  }

/**
 * @brief Checks the requirement `req`.
 *
 * @throws An instance of type `Exc` if the requirement failure.
 */
#define DMITIGR_REQUIRE2(req, Exc) {                \
    if (!(req)) {                                   \
      DMITIGR_THROW_REQUIREMENT_VIOLATED(req, Exc)  \
    }                                               \
  }

/**
 * @brief Checks the requirement `req`.
 *
 * @throws An instance of type `Exc` initialized by `msg`
 * if the requirement failure.
 */
#define DMITIGR_REQUIRE3(req, Exc, msg) {       \
    if (!(req)) {                               \
      throw Exc{msg};                           \
    }                                           \
  }

/**
 * @brief Expands to `macro_name`.
 */
#define DMITIGR_REQUIRE_NAME__(_1, _2, _3, macro_name, ...) macro_name

/**
 * @brief Expands to
 *   - DMITIGR_REQUIRE2(req, Exc) if 2 arguments passed;
 *   - DMITIGR_REQUIRE3(req, Exc, msg) if 3 arguments passed.
 *
 * @remarks The dummy argument `ARG` is used to avoid the warning that ISO
 * C++11 requires at least one argument for the "..." in a variadic macro.
 */
#define DMITIGR_REQUIRE(...) DMITIGR_EXPAND(DMITIGR_REQUIRE_NAME__(__VA_ARGS__, DMITIGR_REQUIRE3, DMITIGR_REQUIRE2, ARG)(__VA_ARGS__))

#endif  // DMITIGR_UTIL_DEBUG_HPP
