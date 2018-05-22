// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_EXCEPTIONS_HXX
#define DMITIGR_PGFE_EXCEPTIONS_HXX

#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <memory>

namespace dmitigr::pgfe::detail {

class iClient_exception : public Client_exception {
public:
  explicit iClient_exception(const Client_errc errc)
    : Client_exception(errc)
  {}

  iClient_exception(const Client_errc errc, const std::string& what)
    : Client_exception(errc, what)
  {}
};

class iServer_exception : public Server_exception {
public:
  explicit iServer_exception(std::shared_ptr<Error> error)
    : Server_exception(error->code())
    , error_(std::move(error))
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  const Error* error() const noexcept override
  {
    return error_.get();
  }

private:
  bool is_invariant_ok()
  {
    return bool(error_);
  }

  std::shared_ptr<Error> error_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_EXCEPTIONS_HXX
