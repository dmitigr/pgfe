// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_DEBUG_HXX
#define DMITIGR_PGFE_INTERNAL_DEBUG_HXX

#include "dmitigr/pgfe/internal/macros.hxx"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace dmitigr::internal {

#ifdef NDEBUG
constexpr bool is_debug_enabled = false;
#else
constexpr bool is_debug_enabled = true;
#endif

} // namespace dmitigr::internal

#define DMINT_DOUT__(...) {                                 \
    std::fprintf(stderr, "Debug output from " __FILE__ ":"  \
      DMINT_XSTR__(__LINE__) ": " __VA_ARGS__);             \
  }

#define DMINT_ASSERT__(a, t) {                                          \
    if (!(a)) {                                                         \
      DMINT_DOUT__("assertion '%s' failed\n", #a)                       \
        if constexpr (t) {                                              \
          throw std::logic_error(std::string("assertion '" #a "' failed at " __FILE__ ":") \
            .append(std::to_string(int(__LINE__))));                    \
        }                                                               \
    }                                                                   \
  }

#define DMINT_DOUT_ALWAYS(...)      DMINT_DOUT__(__VA_ARGS__)
#define DMINT_DOUT_ASSERT_ALWAYS(a) DMINT_ASSERT__(a, false)
#define DMINT_ASSERT_ALWAYS(a)      DMINT_ASSERT__(a, true)

#define DMINT_IF_DEBUG__(code) if constexpr (dmitigr::internal::is_debug_enabled) { code }

#define DMINT_DOUT(...)      { DMINT_IF_DEBUG__(DMINT_DOUT_ALWAYS(__VA_ARGS__)) }
#define DMINT_DOUT_ASSERT(a) { DMINT_IF_DEBUG__(DMINT_DOUT_ASSERT_ALWAYS(a)) }
#define DMINT_ASSERT(a)      { DMINT_IF_DEBUG__(DMINT_ASSERT_ALWAYS(a)) }

// -----------------------------------------------------------------------------

#define DMINT_REQUIRE__(r) {                                            \
    if (!(r)) {                                                         \
      std::string message{"API requirement '" #r "' violated"};         \
      if constexpr (dmitigr::internal::is_debug_enabled) {              \
        message.append(" at " __FILE__ ":").append(std::to_string(int(__LINE__))); \
      }                                                                 \
      throw std::logic_error(message);                                  \
    }                                                                   \
  }

#define DMINT_REQUIRE(r) DMINT_REQUIRE__(r)

#endif  // DMITIGR_PGFE_INTERNAL_DEBUG_HXX
