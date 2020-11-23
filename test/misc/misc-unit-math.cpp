// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/math.hpp>
#include <dmitigr/misc/testo.hpp>

#include <array>
#include <vector>

int main(int, char* argv[])
{
  namespace math = dmitigr::math;
  using namespace dmitigr::testo;

  try {
    // avg
    {
      constexpr auto a1 = math::avg(std::array<int, 5>{1,2,3,4,5});
      static_assert(static_cast<int>(a1) == static_cast<int>(3));
      const auto a2 = math::avg(std::vector<int>{1,2,3,4,5});
      ASSERT(a2 == static_cast<int>(3));
    }

    // dispersion 1
    {
      constexpr auto d1 = math::dispersion(std::array<int, 5>{1,2,3,4,5});
      static_assert(static_cast<int>(d1) == static_cast<int>(2));
      const auto d2 = math::dispersion(std::vector<int>{1,2,3,4,5});
      ASSERT(d2 == static_cast<int>(2));
      constexpr auto d3 = math::dispersion(std::array<int, 5>{600,470,170,430,300});
      static_assert(static_cast<int>(d3) == static_cast<int>(21704));
    }

    // dispersion 2
    {
      constexpr auto d1 = math::dispersion(std::array<int, 5>{1,2,3,4,5}, false);
      static_assert(static_cast<int>(d1) == static_cast<int>(2));
      const auto d2 = math::dispersion(std::vector<int>{1,2,3,4,5});
      ASSERT(d2 == static_cast<int>(2));
      constexpr auto d3 = math::dispersion(std::array<int, 5>{600,470,170,430,300});
      static_assert(static_cast<int>(d3) == static_cast<int>(21704));
    }

    // -------------------------------------------------------------------------
    // Interval
    // -------------------------------------------------------------------------

    using math::Interval;
    using math::Interval_type;

    {
      Interval<int> i;
      ASSERT(i.type() == Interval_type::closed);
      ASSERT(i.min() == 0);
      ASSERT(i.max() == 0);
    }
    {
      Interval<char> i{Interval_type::ropen, 0, 3};
      ASSERT(i.type() == Interval_type::ropen);
      ASSERT(i.min() == 0);
      ASSERT(i.max() == 3);
      ASSERT(!i.has(-1));
      ASSERT(i.has(0));
      ASSERT(i.has(1));
      ASSERT(i.has(2));
      ASSERT(!i.has(3));
    }
    {
      Interval<unsigned> i{Interval_type::lopen, 0, 3};
      ASSERT(i.type() == Interval_type::lopen);
      ASSERT(i.min() == 0);
      ASSERT(i.max() == 3);
      ASSERT(!i.has(-1));
      ASSERT(!i.has(0));
      ASSERT(i.has(1));
      ASSERT(i.has(2));
      ASSERT(i.has(3));
      ASSERT(!i.has(4));
      const auto [min, max] = i.release();
      ASSERT(min == 0);
      ASSERT(max == 3);
      ASSERT(i.type() == Interval_type::closed);
      ASSERT(i.min() == 0);
      ASSERT(i.max() == 0);
    }
    {
      Interval i{Interval_type::open, .0f, 1.0f};
      ASSERT(i.type() == Interval_type::open);
      ASSERT(!i.has(-.3f));
      ASSERT(i.has(.3f));
      ASSERT(!i.has(1.3f));
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
