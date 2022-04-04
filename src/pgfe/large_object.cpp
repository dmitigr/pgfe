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

#include "connection.hpp"
#include "exceptions.hpp"
#include "large_object.hpp"

#include <algorithm>
#include <limits>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Large_object::~Large_object() noexcept
{
  if (is_valid()) {
    auto* const conn = state_->connection_;
    auto [p, e] = conn->registered_lo(state_->id_);
    DMITIGR_ASSERT(p != e);
    state_ = nullptr;
    DMITIGR_ASSERT((*p).use_count() == 1);
    conn->unregister_lo(p);
    DMITIGR_ASSERT(!is_valid());
  }
}

DMITIGR_PGFE_INLINE Large_object::Large_object(std::shared_ptr<State> state)
  : state_{std::move(state)}
{
  DMITIGR_ASSERT(is_valid());
}

DMITIGR_PGFE_INLINE Large_object::Large_object(Large_object&& rhs) noexcept
  : state_{std::move(rhs.state_)}
{}

DMITIGR_PGFE_INLINE Large_object& Large_object::assign(Large_object&& rhs)
{
  if (is_valid())
    throw Client_exception{"cannot assign large object"};

  if (this != &rhs) {
    Large_object tmp{std::move(rhs)};
    DMITIGR_ASSERT(!rhs);
    swap(tmp);
  }

  return *this;
}

DMITIGR_PGFE_INLINE void Large_object::swap(Large_object& rhs) noexcept
{
  using std::swap;
  swap(state_, rhs.state_);
}

DMITIGR_PGFE_INLINE bool Large_object::is_valid() const noexcept
{
  return state_ && state_->connection_ &&
    state_->connection_->pipeline_status() == Pipeline_status::disabled &&
    state_->desc_ >= 0;
}

DMITIGR_PGFE_INLINE bool Large_object::close() noexcept
{
  bool result{true};
  if (is_valid()) {
    result = state_->connection_->close(*this);
    state_ = nullptr;
  }
  DMITIGR_ASSERT(!is_valid());
  return result;
}

DMITIGR_PGFE_INLINE std::int_fast64_t
Large_object::seek(const std::int_fast64_t offset, const Seek_whence whence)
{
  if (!is_valid())
    throw Client_exception{"cannot seek large object"};
  return state_->connection_->seek(*this, offset, whence);
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::tell()
{
  if (!is_valid())
    throw Client_exception{"cannot tell large object"};
  return state_->connection_->tell(*this);
}

DMITIGR_PGFE_INLINE void Large_object::truncate(const std::int_fast64_t new_size)
{
  if (!(is_valid() && new_size >= 0))
    throw Client_exception{"cannot truncate large object"};
  state_->connection_->truncate(*this, new_size);
}

DMITIGR_PGFE_INLINE std::size_t
Large_object::read(char* const buf, const std::size_t size)
{
  if (!(is_valid() && buf && size <= std::numeric_limits<int>::max()))
    throw Client_exception{"cannot read large object"};
  return static_cast<std::size_t>(state_->connection_->read(*this, buf, size));
}

DMITIGR_PGFE_INLINE std::size_t
Large_object::write(const char* const buf, const std::size_t size)
{
  if (!(is_valid() && buf && size <= std::numeric_limits<int>::max()))
    throw Client_exception{"cannot write large object"};
  return static_cast<std::size_t>(state_->connection_->write(*this, buf, size));
}

DMITIGR_PGFE_INLINE const Connection& Large_object::connection() const
{
  if (!is_valid())
    throw Client_exception{"cannot get connection of invalid large object"};
  return *state_->connection_;
}

DMITIGR_PGFE_INLINE Connection& Large_object::connection()
{
  return const_cast<Connection&>(static_cast<const Large_object*>(this)->connection());
}

DMITIGR_PGFE_INLINE int Large_object::descriptor() const noexcept
{
  return is_valid() ? state_->desc_ : -1;
}

} // namespace dmitigr::pgfe
