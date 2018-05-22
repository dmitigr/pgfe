// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_NET_INET_HXX
#define DMITIGR_PGFE_INTERNAL_NET_INET_HXX

#include <string>

namespace dmitigr::internal::net {

/**
 * @internal
 *
 * @returns `true` if the `address` denotes a valid IPv4 or IPv6 address, or
 * `false` otherwise.
 */
bool is_ip_address_valid(const std::string& address);

/**
 * @internal
 *
 * @returns `true` if the `domain_name` denotes a valid domain name, or
 * `false` otherwise.
 */
bool is_domain_name_valid(const std::string& domain_name);

} // namespace dmitigr::internal::net

#endif  // DMITIGR_PGFE_INTERNAL_NET_INET_HXX
