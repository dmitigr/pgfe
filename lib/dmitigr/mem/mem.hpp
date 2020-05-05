// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
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

#ifndef DMITIGR_MEM_MEM_HPP
#define DMITIGR_MEM_MEM_HPP

#include "dmitigr/mem/types_fwd.hpp"
#include "dmitigr/mem/version.hpp"

#include <memory>

namespace dmitigr::mem {

/**
 * @brief A custom deleter for smart pointers.
 */
template<typename T>
class Conditional_delete final {
public:
  /**
   * @brief The default constructor.
   *
   * @par Effects
   * `condition()`.
   */
  constexpr Conditional_delete() noexcept = default;

  /**
   * @brief The constructor.
   *
   * @par Effects
   * `condition()`.
   */
  explicit constexpr Conditional_delete(const bool condition) noexcept
    : condition_(condition)
  {}

  /**
   * @returns The value of condition.
   */
  constexpr bool condition() const noexcept
  {
    return condition_;
  }

  /**
   * @brief Applies `std::default_delete::operator()` to the
   * pointer `o` if and only if the `(condition() == true)`.
   */
  void operator()(T* const o) const noexcept
  {
    if (condition())
      std::default_delete<T>{}(o);
  }

private:
  bool condition_{true};
};

} // namespace dmitigr::mem

#endif  // DMITIGR_MEM_MEM_HPP
