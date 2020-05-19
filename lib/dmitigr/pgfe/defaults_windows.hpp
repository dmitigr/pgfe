// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_DEFAULTS_HPP
#define DMITIGR_PGFE_DEFAULTS_HPP

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file was generated automatically. Edit defaults.hpp.in instead!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "dmitigr/pgfe/basics.hpp"
#include <dmitigr/base/filesystem.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

/**
 * @brief Defaults which are sets at build time.
 */
namespace dmitigr::pgfe::detail::defaults {

constexpr const Communication_mode communication_mode{Communication_mode::net};

constexpr const std::optional<std::chrono::milliseconds> connect_timeout{};
constexpr const std::optional<std::chrono::milliseconds> wait_response_timeout{};
constexpr const std::optional<std::chrono::milliseconds> wait_last_response_timeout{};

#ifndef _WIN32
const std::filesystem::path      uds_directory{""};
const std::optional<std::string> uds_require_server_process_username{};
#endif

constexpr const bool                                tcp_keepalives_enabled{};
constexpr const std::optional<std::chrono::seconds> tcp_keepalives_idle{};
constexpr const std::optional<std::chrono::seconds> tcp_keepalives_interval{};
constexpr const std::optional<int>                  tcp_keepalives_count{};

const std::optional<std::string>  net_address{};
const std::optional<std::string>  net_hostname{"localhost"};
constexpr const std::int_fast32_t port{5432};

const std::string                username{"postgres"};
const std::string                database{"postgres"};
const std::optional<std::string> password{};
const std::optional<std::string> kerberos_service_name{};

constexpr const bool ssl_enabled{};
constexpr const bool ssl_server_hostname_verification_enabled{};
constexpr const bool ssl_compression_enabled{};
const std::optional<std::filesystem::path> ssl_certificate_file{};
const std::optional<std::filesystem::path> ssl_private_key_file{};
const std::optional<std::filesystem::path> ssl_certificate_authority_file{};
const std::optional<std::filesystem::path> ssl_certificate_revocation_list_file{};

} // namespace dmitigr::pgfe::detail::defaults

#endif // DMITIGR_PGFE_DEFAULTS_HPP
