// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/rng.hpp>
#include <dmitigr/misc/testo.hpp>
#include <dmitigr/misc/uuid.hpp>

int main(int, char* argv[])
{
  namespace rng = dmitigr::rng;
  namespace uuid = dmitigr::uuid;
  using namespace dmitigr::testo;

  try {
    rng::seed_by_now();
    const auto u = uuid::Uuid::make_v4();
    const auto s = u.to_string();
    ASSERT(s.size() == 36);
    std::cout << s << std::endl;
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
