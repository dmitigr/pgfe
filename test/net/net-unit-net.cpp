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

#include "../../src/base/assert.hpp"
#include "../../src/net/net.hpp"

int main()
{
  try {
    namespace net = dmitigr::net;

    const std::string v4_addr_str{"192.168.1.2"};
    DMITIGR_ASSERT(net::Ip_address::is_valid(v4_addr_str));

    auto ip = net::Ip_address::from_text(v4_addr_str);
    DMITIGR_ASSERT(ip);
    DMITIGR_ASSERT(ip.is_valid());
    DMITIGR_ASSERT(ip.family() == net::Protocol_family::ipv4);
    DMITIGR_ASSERT(ip.binary());
    DMITIGR_ASSERT(ip.to_string() == v4_addr_str);

    const std::string invalid_v4_addr_str{"256.168.1.2"};
    DMITIGR_ASSERT(!net::Ip_address::is_valid(invalid_v4_addr_str));

    const std::string v6_addr_str{"fe80::1:2:3:4"};
    ip = net::Ip_address::from_text(v6_addr_str);
    DMITIGR_ASSERT(ip.family() == net::Protocol_family::ipv6);
    DMITIGR_ASSERT(ip.binary());
    DMITIGR_ASSERT(ip.to_string() == v6_addr_str);

    const int n = 10;
    auto n1 = net::conv(n);
    DMITIGR_ASSERT(n != n1);
    n1 = net::conv(n1);
    DMITIGR_ASSERT(n == n1);

    const float f = 123.456;
    auto f1 = net::conv(f);
    DMITIGR_ASSERT(f != f1);
    f1 = net::conv(f1);
    DMITIGR_ASSERT(f == f1);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
