// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/large_object.hpp"

#include <libpq-fe.h>

#include <limits>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Large_object::~Large_object()
{
  try {
    close();
  } catch (...) {}
}

DMITIGR_PGFE_INLINE Large_object::Large_object(Connection* const conn, const int desc) noexcept
  : conn_{conn}
  , desc_{desc}
{}

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
  if (!is_valid())
    return false;
  else if (conn_->close(*this)) {
    *this = Large_object{};
    return true;
  } else
    return false;
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::seek(const std::int_fast64_t offset, const Seek_whence whence) noexcept
{
  assert(is_valid());
  return conn_->seek(*this, offset, whence);
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::tell() noexcept
{
  assert(is_valid());
  return conn_->tell(*this);
}

DMITIGR_PGFE_INLINE bool Large_object::truncate(const std::int_fast64_t new_size) noexcept
{
  assert(is_valid());
  assert(new_size >= 0);
  return conn_->truncate(*this, new_size);
}

int Large_object::read(char* const buf, const std::size_t size) noexcept
{
  assert(is_valid());
  assert(buf && size <= std::numeric_limits<int>::max());
  return conn_->read(*this, buf, size);
}

int Large_object::write(const char* const buf, const std::size_t size) noexcept
{
  assert(is_valid());
  assert(buf && size <= std::numeric_limits<int>::max());
  return conn_->write(*this, buf, size);
}

} // namespace dmitigr::pgfe
