// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
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

#include "connection.hpp"
#include "exceptions.hpp"
#include "large_object.hpp"

#include <limits>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Large_object::Large_object(Connection* const conn,
  const int desc)
  : conn_{conn}
  , desc_{desc}
{
  DMITIGR_ASSERT(conn && (desc >= 0) &&
    (conn->pipeline_status() == Pipeline_status::disabled));
}

DMITIGR_PGFE_INLINE Large_object::Large_object(Large_object&& rhs) noexcept
  : conn_{rhs.conn_}
  , desc_{rhs.desc_}
{
  rhs.conn_ = {};
  rhs.desc_ = -1;
}

DMITIGR_PGFE_INLINE Large_object& Large_object::operator=(Large_object&& rhs) noexcept
{
  if (this != &rhs) {
    Large_object tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Large_object::swap(Large_object& rhs) noexcept
{
  using std::swap;
  swap(conn_, rhs.conn_);
  swap(desc_, rhs.desc_);
}

DMITIGR_PGFE_INLINE bool Large_object::is_valid() const noexcept
{
  return conn_ && (desc_ >= 0);
}

DMITIGR_PGFE_INLINE bool Large_object::close() noexcept
{
  bool result{true};
  if (is_valid()) {
    result = conn_->close(*this);
    conn_ = {};
    desc_ = -1;
  }
  DMITIGR_ASSERT(!is_valid());
  return result;
}

DMITIGR_PGFE_INLINE std::int_fast64_t
Large_object::seek(const std::int_fast64_t offset, const Seek_whence whence)
{
  if (!is_valid())
    throw Client_exception{"cannot seek large object"};
  return conn_->seek(*this, offset, whence);
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::tell()
{
  if (!is_valid())
    throw Client_exception{"cannot tell large object"};
  return conn_->tell(*this);
}

DMITIGR_PGFE_INLINE bool Large_object::truncate(const std::int_fast64_t new_size)
{
  if (!(is_valid() && new_size >= 0))
    throw Client_exception{"cannot truncate large object"};
  return conn_->truncate(*this, new_size);
}

DMITIGR_PGFE_INLINE int Large_object::read(char* const buf, const std::size_t size)
{
  if (!(is_valid() && buf && size <= std::numeric_limits<int>::max()))
    throw Client_exception{"cannot read large object"};
  return conn_->read(*this, buf, size);
}

DMITIGR_PGFE_INLINE int Large_object::write(const char* const buf,
  const std::size_t size)
{
  if (!(is_valid() && buf && size <= std::numeric_limits<int>::max()))
    throw Client_exception{"cannot write large object"};
  return conn_->write(*this, buf, size);
}

DMITIGR_PGFE_INLINE const Connection* Large_object::connection() const noexcept
{
  return conn_;
}

DMITIGR_PGFE_INLINE Connection* Large_object::connection() noexcept
{
  return const_cast<Connection*>(static_cast<const Large_object*>(this)->connection());
}

DMITIGR_PGFE_INLINE int Large_object::descriptor() const noexcept
{
  return desc_;
}

} // namespace dmitigr::pgfe
