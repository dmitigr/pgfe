// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/str.hpp>
#include <dmitigr/misc/testo.hpp>

int main(int, char* argv[])
{
  namespace str = dmitigr::str;
  using namespace dmitigr::testo;

  try {
    // -------------------------------------------------------------------------
    // trim
    // -------------------------------------------------------------------------

    // Empty string
    {
      std::string s;
      str::trim(s);
      ASSERT(s.empty());
    }

    // String with only spaces
    {
      std::string s{" \f\n\r\t\v"};
      str::trim(s);
      ASSERT(s.empty());
    }

    // String without spaces
    {
      std::string s{"content"};
      str::trim(s);
      ASSERT(s == "content");
    }

    // String with spaces from left
    {
      std::string s{" \f\n\r\t\vcontent"};
      str::trim(s);
      ASSERT(s == "content");
    }

    // String with spaces from right
    {
      std::string s{"content \f\n\r\t\v"};
      str::trim(s);
      ASSERT(s == "content");
    }

    // String with spaces from both sides
    {
      std::string s{" \f\n\r\t\vcontent \f\n\r\t\v"};
      str::trim(s);
      ASSERT(s == "content");
    }

    // String with spaces from both sides with spaces in the content
    {
      std::string s{" \f\n\r\t\vcon ten t \f\n\r\t\v"};
      str::trim(s);
      ASSERT(s == "con ten t");
    }

    // -------------------------------------------------------------------------
    // split
    // -------------------------------------------------------------------------

    // Emptry string, no separators
    {
      std::string s;
      const auto v = str::split(s, "");
      ASSERT(s.empty());
    }

    // Emptry string and separator
    {
      std::string s;
      const auto v = str::split(s, ",");
      ASSERT(s.empty());
    }

    // String with only separator
    {
      std::string s{","};
      const auto v = str::split(s, s);
      ASSERT(v.size() == 2);
    }

    // String with only separators
    {
      std::string s{",,..!!"};
      const auto v = str::split(s, s);
      ASSERT(v.size() == 7);
    }

    // String without separator
    {
      std::string s{"content"};
      const auto v = str::split(s, ",");
      ASSERT(v.size() == 1);
    }

    // String with separator
    {
      std::string s{"1 2 3"};
      const auto v = str::split(s, " ");
      ASSERT(v.size() == 3);
      ASSERT(v[0] == "1");
      ASSERT(v[1] == "2");
      ASSERT(v[2] == "3");
    }

    // String with separators
    {
      std::string s{"1 2,3"};
      const auto v = str::split(s, " ,");
      ASSERT(v.size() == 3);
      ASSERT(v[0] == "1");
      ASSERT(v[1] == "2");
      ASSERT(v[2] == "3");
    }

    // String with separators to vector of string_view
    {
      std::string s{"1 2,3"};
      const auto v = str::split<std::string_view>(s, " ,");
      ASSERT(v.size() == 3);
      ASSERT(v[0] == "1");
      ASSERT(v[1] == "2");
      ASSERT(v[2] == "3");
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
