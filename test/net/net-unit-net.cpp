// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#include <dmitigr/net.hpp>
#include <dmitigr/testo.hpp>

int main(int, char* argv[])
{
  namespace net = dmitigr::net;
  using namespace dmitigr::testo;

  try {
    const std::string v4_addr_str{"192.168.1.2"};
    ASSERT(net::Ip_address::is_valid(v4_addr_str));

    auto ip = net::Ip_address::make(v4_addr_str);
    ASSERT(ip->family() == net::Ip_version::v4);
    ASSERT(ip->binary());
    ASSERT(ip->to_string() == v4_addr_str);

    const std::string invalid_v4_addr_str{"256.168.1.2"};
    ASSERT(!net::Ip_address::is_valid(invalid_v4_addr_str));

    const std::string v6_addr_str{"fe80::1:2:3:4"};
    ip = net::Ip_address::make(v6_addr_str);
    ASSERT(ip->family() == net::Ip_version::v6);
    ASSERT(ip->binary());
    ASSERT(ip->to_string() == v6_addr_str);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
  return 0;
}
