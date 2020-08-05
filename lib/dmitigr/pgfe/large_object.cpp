// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/base/debug.hpp"
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

DMITIGR_PGFE_INLINE Large_object::Large_object(Connection* const conn, const int desc)
  : conn_{conn}
  , desc_{desc}
{}

DMITIGR_PGFE_INLINE Large_object::Large_object(Large_object&& rhs)
  : conn_{rhs.conn_}
  , desc_{rhs.desc_}
{
  rhs.conn_ = {};
  rhs.desc_ = -1;
}

DMITIGR_PGFE_INLINE Large_object& Large_object::operator=(Large_object&& rhs)
{
  Large_object tmp{std::move(rhs)};
  swap(tmp);
  return *this;
}

DMITIGR_PGFE_INLINE void Large_object::swap(Large_object& rhs) noexcept
{
  std::swap(conn_, rhs.conn_);
  std::swap(desc_, rhs.desc_);
}

DMITIGR_PGFE_INLINE bool Large_object::is_valid() const noexcept
{
  return conn_ && (desc_ >= 0);
}

DMITIGR_PGFE_INLINE bool Large_object::close()
{
  if (!is_valid())
    return false;
  else if (conn_->close(*this)) {
    *this = Large_object{};
    return true;
  } else
    return false;
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::seek(const std::int_fast64_t offset, const Seek_whence whence)
{
  DMITIGR_REQUIRE(is_valid(), std::logic_error);
  return conn_->seek(*this, offset, whence);
}

DMITIGR_PGFE_INLINE std::int_fast64_t Large_object::tell()
{
  DMITIGR_REQUIRE(is_valid(), std::logic_error);
  return conn_->tell(*this);
}

DMITIGR_PGFE_INLINE bool Large_object::truncate(const std::int_fast64_t new_size)
{
  DMITIGR_REQUIRE(new_size >= 0, std::invalid_argument);
  DMITIGR_REQUIRE(is_valid(), std::logic_error);
  return conn_->truncate(*this, new_size);
}

int Large_object::read(char* const buf, const std::size_t size)
{
  DMITIGR_REQUIRE(buf && size <= std::numeric_limits<int>::max(), std::invalid_argument);
  DMITIGR_REQUIRE(is_valid(), std::logic_error);
  return conn_->read(*this, buf, size);
}

int Large_object::write(const char* const buf, const std::size_t size)
{
  DMITIGR_REQUIRE(buf && size <= std::numeric_limits<int>::max(), std::invalid_argument);
  DMITIGR_REQUIRE(is_valid(), std::logic_error);
  return conn_->write(*this, buf, size);
}

} // namespace dmitigr::pgfe
