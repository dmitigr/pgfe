// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_ERRC_HPP
#define DMITIGR_STR_ERRC_HPP

namespace dmitigr::str {

/**
 * @brief A read error code.
 */
enum class Errc {
  success = 0,
  stream_error = 1,
  invalid_input = 2
};

} // namespace dmitigr::str

#endif  // DMITIGR_STR_ERRC_HPP
