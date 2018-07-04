// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_DEBUG_HXX
#define DMITIGR_PGFE_INTERNAL_DEBUG_HXX

#include "dmitigr/pgfe/internal/macros.hxx"

#include <cstdio>
#include <stdexcept>
#include <string>

namespace dmitigr::pgfe::internal {

#ifdef NDEBUG
constexpr bool is_debug_enabled = false;
#else
constexpr bool is_debug_enabled = true;
#endif

} // namespace dmitigr::pgfe::internal

#define DMITIGR_PGFE_INTERNAL_DOUT__(...) {                     \
    std::fprintf(stderr, "Debug output from " __FILE__ ":"      \
      DMITIGR_PGFE_INTERNAL_XSTR__(__LINE__) ": " __VA_ARGS__); \
  }

#define DMITIGR_PGFE_INTERNAL_ASSERT__(a, t) {                          \
    if (!(a)) {                                                         \
      DMITIGR_PGFE_INTERNAL_DOUT__("assertion '%s' failed\n", #a)       \
        if constexpr (t) {                                              \
          throw std::logic_error(std::string("assertion '" #a "' failed at " __FILE__ ":") \
            .append(std::to_string(int(__LINE__))));                    \
        }                                                               \
    }                                                                   \
  }

#define DMITIGR_PGFE_INTERNAL_DOUT_ALWAYS(...)      DMITIGR_PGFE_INTERNAL_DOUT__(__VA_ARGS__)
#define DMITIGR_PGFE_INTERNAL_DOUT_ASSERT_ALWAYS(a) DMITIGR_PGFE_INTERNAL_ASSERT__(a, false)
#define DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(a)      DMITIGR_PGFE_INTERNAL_ASSERT__(a, true)

#define DMITIGR_PGFE_INTERNAL_IF_DEBUG__(code) if constexpr (dmitigr::pgfe::internal::is_debug_enabled) { code }

#define DMITIGR_PGFE_INTERNAL_DOUT(...)      { DMITIGR_PGFE_INTERNAL_IF_DEBUG__(DMITIGR_PGFE_INTERNAL_DOUT_ALWAYS(__VA_ARGS__)) }
#define DMITIGR_PGFE_INTERNAL_DOUT_ASSERT(a) { DMITIGR_PGFE_INTERNAL_IF_DEBUG__(DMITIGR_PGFE_INTERNAL_DOUT_ASSERT_ALWAYS(a)) }
#define DMITIGR_PGFE_INTERNAL_ASSERT(a)      { DMITIGR_PGFE_INTERNAL_IF_DEBUG__(DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(a)) }

// -----------------------------------------------------------------------------

#define DMITIGR_PGFE_INTERNAL_REQUIRE__(r) {                            \
    if (!(r)) {                                                         \
      std::string message{"API requirement '" #r "' violated"};         \
      if constexpr (dmitigr::pgfe::internal::is_debug_enabled) {        \
        message.append(" at " __FILE__ ":").append(std::to_string(int(__LINE__))); \
      }                                                                 \
      throw std::logic_error(message);                                  \
    }                                                                   \
  }

#define DMITIGR_PGFE_INTERNAL_REQUIRE(r) DMITIGR_PGFE_INTERNAL_REQUIRE__(r)

#endif  // DMITIGR_PGFE_INTERNAL_DEBUG_HXX
