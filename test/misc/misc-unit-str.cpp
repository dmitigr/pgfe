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
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
