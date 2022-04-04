// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../../src/base/assert.hpp"
#include "../../src/str/transform.hpp"

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
