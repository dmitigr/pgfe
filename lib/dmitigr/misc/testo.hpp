// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or testo.hpp

#ifndef DMITIGR_MISC_TESTO_HPP
#define DMITIGR_MISC_TESTO_HPP

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>

#ifndef ASSERT
#define ASSERT(a) assert(a)
#endif

namespace dmitigr::testo {

/// @returns `true` if instance of type `E` is thrown upon calling of `f`.
template<class E, typename F>
bool is_throw_works(F&& f) noexcept
{
  try {
    f();
  } catch (const E&) {
    return true;
  } catch (...) {}
  return false;
}

/// Pretty-prints `e.what()`.
inline void report_failure(const std::string_view test_name, const std::exception& e)
{
  std::cerr << "Test \"" << test_name.data() << "\" failed (std::exception catched): " << e.what() << std::endl;
}

/// @overload
inline void report_failure(const std::string_view test_name)
{
  std::cerr << "Test \"" << test_name.data() << "\" failed (unknown exception catched)" << std::endl;
}

/// @returns The duration of call of `f`.
template<typename D = std::chrono::milliseconds, typename F>
auto time(F&& f)
{
  namespace chrono = std::chrono;
  const auto start = chrono::high_resolution_clock::now();
  f();
  const auto end = chrono::high_resolution_clock::now();
  return chrono::duration_cast<D>(end - start);
}

} // namespace dmitigr::testo

#endif // DMITIGR_MISC_TESTO_HPP
