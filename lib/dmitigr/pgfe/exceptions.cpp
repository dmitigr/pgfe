// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/common/debug.hpp>

#include <memory>

namespace dmitigr::pgfe::detail {

class iClient_exception final : public Client_exception {
public:
  explicit iClient_exception(const Client_errc errc)
    : Client_exception(errc)
  {}

  iClient_exception(const Client_errc errc, const std::string& what)
    : Client_exception(errc, what)
  {}
};

class iServer_exception final : public Server_exception {
public:
  explicit iServer_exception(std::shared_ptr<Error> error)
    : Server_exception(error->code())
    , error_(std::move(error))
  {
    DMITIGR_ASSERT(is_invariant_ok());
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

#include "dmitigr/pgfe/implementation_footer.hpp"
