// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection.hxx"
#include "dmitigr/pgfe/connection_options.hxx"

#include <libpq-fe.h>

#include <atomic>

namespace dmitigr::pgfe {

namespace detail {

std::unique_ptr<Connection> iConnection_options::make_connection() const
{
  return std::make_unique<detail::pq_Connection>(*this);
}

} // namespace detail

DMITIGR_PGFE_API std::unique_ptr<Connection> Connection::make(const Connection_options* const options)
{
  if (options)
    return options->make_connection();
  else
    return detail::iConnection_options{}.make_connection();
}

} // namespace dmitigr::pgfe

namespace pgfe = dmitigr::pgfe;

namespace {

std::atomic<bool>& openssl_library_initialization_flag()
{
  static std::atomic<bool> result{true};
  return result;
}

std::atomic<bool>& crypto_library_initialization_flag()
{
  static std::atomic<bool> result{true};
  return result;
}

} // namespace

DMITIGR_PGFE_API void pgfe::set_openssl_library_initialization_enabled(const bool value)
{
  openssl_library_initialization_flag() = value;
  ::PQinitOpenSSL(openssl_library_initialization_flag(), crypto_library_initialization_flag());
}

DMITIGR_PGFE_API bool pgfe::is_openssl_library_initialization_enabled()
{
  return openssl_library_initialization_flag();
}

DMITIGR_PGFE_API void pgfe::set_crypto_library_initialization_enabled(const bool value)
{
  crypto_library_initialization_flag() = value;
  ::PQinitOpenSSL(openssl_library_initialization_flag(), crypto_library_initialization_flag());
}

DMITIGR_PGFE_API bool pgfe::is_crypto_library_initialization_enabled()
{
  return crypto_library_initialization_flag();
}
