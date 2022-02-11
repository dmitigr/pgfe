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

#ifndef DMITIGR_PGFE_DEFAULTS_HPP
#define DMITIGR_PGFE_DEFAULTS_HPP

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file was generated automatically. Edit defaults.hpp.in instead!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "basics.hpp"
#include "../fs/filesystem.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

/// Defaults which are sets at build time.
namespace dmitigr::pgfe::detail::defaults {

constexpr const Communication_mode communication_mode{Communication_mode::uds};

constexpr const std::optional<std::chrono::milliseconds> connect_timeout{10000};
constexpr const std::optional<std::chrono::milliseconds> wait_response_timeout{};

const std::filesystem::path      uds_directory{"C:/Temp"};
const std::optional<std::string> uds_require_server_process_username{};

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
