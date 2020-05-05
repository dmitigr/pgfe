// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or base.hpp

#ifndef DMITIGR_BASE_MACROS_HPP
#define DMITIGR_BASE_MACROS_HPP

/**
 * @brief Stringizes the argument `s`.
 */
#define DMITIGR_STRINGIZED(s) #s

/**
 * @brief X-stringizes the argument `s`.
 */
#define DMITIGR_XSTRINGIZED(s) DMITIGR_STRINGIZED(s)

/**
 * @brief Expands to `x`.
 */
#define DMITIGR_EXPAND(x) x

#endif // DMITIGR_BASE_MACROS_HPP
