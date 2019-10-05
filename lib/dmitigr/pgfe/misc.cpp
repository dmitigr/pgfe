// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/misc.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <libpq-fe.h>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE void set_initialization(const External_library library)
{
  const auto libssl = static_cast<bool>(library & External_library::libssl);
  const auto libcrypto = static_cast<bool>(library & External_library::libcrypto);
  ::PQinitOpenSSL(libssl, libcrypto);
}

} // namespace dmitigr::pgfe

#include "dmitigr/pgfe/implementation_footer.hpp"
