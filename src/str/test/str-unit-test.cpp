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

#include "../../str.hpp"
#include "../../base/assert.hpp"

int main()
{
  try {
    namespace str = dmitigr::str;

    // -------------------------------------------------------------------------
    // trim
    // -------------------------------------------------------------------------

    // Empty string
    {
      std::string s;
      str::trim(s);
      DMITIGR_ASSERT(s.empty());
    }

    // String with only spaces
    {
      std::string s{" \f\n\r\t\v"};
      str::trim(s);
      DMITIGR_ASSERT(s.empty());
    }

    // String without spaces
    {
      std::string s{"content"};
      str::trim(s);
      DMITIGR_ASSERT(s == "content");
    }

    // String with spaces from left
    {
      std::string s{" \f\n\r\t\vcontent"};
      str::trim(s);
      DMITIGR_ASSERT(s == "content");
    }

    // String with spaces from right
    {
      std::string s{"content \f\n\r\t\v"};
      str::trim(s);
      DMITIGR_ASSERT(s == "content");
    }

    // String with spaces from both sides
    {
      std::string s{" \f\n\r\t\vcontent \f\n\r\t\v"};
      str::trim(s);
      DMITIGR_ASSERT(s == "content");
    }

    // String with spaces from both sides with spaces in the content
    {
      std::string s{" \f\n\r\t\vcon ten t \f\n\r\t\v"};
      str::trim(s);
      DMITIGR_ASSERT(s == "con ten t");
    }

    // -------------------------------------------------------------------------
    // split
    // -------------------------------------------------------------------------

    // Emptry string, no separators
    {
      std::string s;
      const auto v = str::split(s, "");
      DMITIGR_ASSERT(s.empty());
    }

    // Emptry string and separator
    {
      std::string s;
      const auto v = str::split(s, ",");
      DMITIGR_ASSERT(s.empty());
    }

    // String with only separator
    {
      std::string s{","};
      const auto v = str::split(s, s);
      DMITIGR_ASSERT(v.size() == 2);
    }

    // String with only separators
    {
      std::string s{",,..!!"};
      const auto v = str::split(s, s);
      DMITIGR_ASSERT(v.size() == 7);
    }

    // String without separator
    {
      std::string s{"content"};
      const auto v = str::split(s, ",");
      DMITIGR_ASSERT(v.size() == 1);
    }

    // String with separator
    {
      std::string s{"1 2 3"};
      const auto v = str::split(s, " ");
      DMITIGR_ASSERT(v.size() == 3);
      DMITIGR_ASSERT(v[0] == "1");
      DMITIGR_ASSERT(v[1] == "2");
      DMITIGR_ASSERT(v[2] == "3");
    }

    // String with separators
    {
      std::string s{"1 2,3"};
      const auto v = str::split(s, " ,");
      DMITIGR_ASSERT(v.size() == 3);
      DMITIGR_ASSERT(v[0] == "1");
      DMITIGR_ASSERT(v[1] == "2");
      DMITIGR_ASSERT(v[2] == "3");
    }

    // String with separators to vector of string_view
    {
      std::string s{"1 2,3"};
      const auto v = str::split<std::string_view>(s, " ,");
      DMITIGR_ASSERT(v.size() == 3);
      DMITIGR_ASSERT(v[0] == "1");
      DMITIGR_ASSERT(v[1] == "2");
      DMITIGR_ASSERT(v[2] == "3");
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
