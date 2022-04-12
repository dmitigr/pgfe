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

#include "../base/assert.hpp"
#include "../os/error.hpp"
#include "completion.hpp"

#include <cassert>
#include <cstdlib>
#include <system_error>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Completion::Completion() noexcept
{
  DMITIGR_ASSERT(!is_valid());
}

DMITIGR_PGFE_INLINE Completion::Completion(Completion&& rhs) noexcept
  : row_count_{rhs.row_count_}
  , tag_{std::move(rhs.tag_)}
{
  rhs.row_count_ = -2;
}

DMITIGR_PGFE_INLINE Completion::Completion(const std::string_view tag)
  : row_count_{-1} // mark instance as valid
{
  DMITIGR_ASSERT(tag.data());

  constexpr char space{' '};
  auto space_before_word_pos = tag.find_last_of(space);
  if (space_before_word_pos != std::string_view::npos) {
    auto end_word_pos = tag.size() - 1;
    while (space_before_word_pos != std::string_view::npos) {
      /*
       * The tag can include affected row count as the last word. We'll try to
       * convert each word of the tag to a number. All numbers except the last
       * one (i.e. affected row count) must be ignored.
       */
      const auto word_size = end_word_pos - space_before_word_pos;
      const std::string word{tag.substr(space_before_word_pos + 1, word_size)};

      errno = 0;
      char* p{};
      const long number = std::strtol(word.c_str(), &p, 10);
      if (const int err = errno)
        throw Client_exception{"cannot parse command completion tag:"
          " " + os::error_message(err)};
      if (p == word.c_str())
        // The word is not a number.
        break;
      else if (row_count_ < 0)
        row_count_ = number;

      end_word_pos = space_before_word_pos - 1;
      space_before_word_pos = tag.find_last_of(space, end_word_pos);
    }
    tag_ = tag.substr(0, end_word_pos + 1);
  } else
    tag_ = tag;

  DMITIGR_ASSERT(is_valid());
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Completion& Completion::operator=(Completion&& rhs) noexcept
{
  if (this != &rhs) {
    Completion tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Completion::swap(Completion& rhs) noexcept
{
  using std::swap;
  swap(row_count_, rhs.row_count_);
  swap(tag_, rhs.tag_);
}

DMITIGR_PGFE_INLINE bool Completion::is_valid() const noexcept
{
  return row_count_ > -2;
}

DMITIGR_PGFE_INLINE const std::string& Completion::tag() const noexcept
{
  return tag_;
}

DMITIGR_PGFE_INLINE std::optional<long> Completion::row_count() const noexcept
{
  return row_count_ >= 0 ? std::optional<long>(row_count_) : std::nullopt;
}

DMITIGR_PGFE_INLINE bool Completion::is_invariant_ok() const noexcept
{
  return (row_count_ < 0) || !tag_.empty();
}

} // namespace dmitigr::pgfe
