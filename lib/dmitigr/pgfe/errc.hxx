// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ERRC_HXX
#define DMITIGR_PGFE_ERRC_HXX

#include "dmitigr/pgfe/errc.hpp"

namespace dmitigr::pgfe::detail {

/*
 * @returns The literal representation of the `errc`.
 */
const char* to_literal(Client_errc errc);

/*
 * @returns The literal representation of the `errc`.
 */
const char* to_literal(Server_errc errc);

} // namespace dmitigr::pgfe::detail

#endif // DMITIGR_PGFE_ERRC_HXX
