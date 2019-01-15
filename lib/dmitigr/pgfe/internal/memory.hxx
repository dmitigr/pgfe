// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_MEMORY_HXX
#define DMITIGR_PGFE_INTERNAL_MEMORY_HXX

#include <memory>

namespace dmitigr::pgfe::internal::memory {

/**
 * @internal
 *
 * @brief This class is intended to use with smart pointers.
 *
 * It applies `std::default_delete::operator()` to the pointer owned by the
 * smart pointer when appropriate if and only if condition, passed to the
 * constructor is `true`.
 */
template<typename T>
class Conditional_delete {
public:
  constexpr Conditional_delete() noexcept = default;

  explicit constexpr Conditional_delete(const bool condition) noexcept
    : condition_(condition)
  {}

  constexpr bool condition() const noexcept
  {
    return condition_;
  }

  void operator()(T* const o) const noexcept
  {
    if (condition_)
      std::default_delete<T>{}(o);
  }

private:
  bool condition_{true};
};

} // namespace dmitigr::pgfe::internal::memory

#endif  // DMITIGR_PGFE_INTERNAL_MEMORY_HXX
