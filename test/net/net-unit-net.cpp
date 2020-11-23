// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/net.hpp>

int main(int, char* argv[])
{
  namespace net = dmitigr::net;
  using namespace dmitigr::testo;

  try {
    const std::string v4_addr_str{"192.168.1.2"};
    ASSERT(net::Ip_address::is_valid(v4_addr_str));

    net::Ip_address ip{v4_addr_str};
    ASSERT(ip.family() == net::Protocol_family::ipv4);
    ASSERT(ip.binary());
    ASSERT(ip.to_string() == v4_addr_str);

    const std::string invalid_v4_addr_str{"256.168.1.2"};
    ASSERT(!net::Ip_address::is_valid(invalid_v4_addr_str));

    const std::string v6_addr_str{"fe80::1:2:3:4"};
    ip = net::Ip_address{v6_addr_str};
    ASSERT(ip.family() == net::Protocol_family::ipv6);
    ASSERT(ip.binary());
    ASSERT(ip.to_string() == v6_addr_str);

    const int n = 10;
    auto n1 = net::conv(n);
    ASSERT(n != n1);
    n1 = net::conv(n1);
    ASSERT(n == n1);

    const float f = 123.456;
    auto f1 = net::conv(f);
    ASSERT(f != f1);
    f1 = net::conv(f1);
    ASSERT(f == f1);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
  return 0;
}
