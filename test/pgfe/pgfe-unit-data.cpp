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

#include "pgfe-unit.hpp"

#include <cstring>
#include <string>
#include <vector>

int main()
  try {
    namespace pgfe = dmitigr::pgfe;
    using pgfe::to;

    // Data::make(std::string_view)
    {
      const std::size_t sz = std::strlen("Dmitry Igrishin");
      const auto d = pgfe::Data::make("Dmitry Igrishin");
      DMITIGR_ASSERT(d->format() == pgfe::Data_format::text);
      DMITIGR_ASSERT(d->size() == sz);
      DMITIGR_ASSERT(to<std::string_view>(*d) == "Dmitry Igrishin");
    }

    // Data::make(std::string_VIEW, Data_format)
    {
      const std::string name{"Dmitry Igrishin"};
      const std::size_t sz = name.size();
      const auto d = pgfe::Data::make(name, pgfe::Data_format::text);
      DMITIGR_ASSERT(d->format() == pgfe::Data_format::text);
      DMITIGR_ASSERT(d->size() == sz);
      DMITIGR_ASSERT(to<std::string_view>(*d) == name);
    }

    // Data::make(std::unique_ptr<void, void (*)(void*)>&&, std::size_t, Data_format)
    {
      char substr[] = {'D', 'm', 'i', 't', '\0'};
      char mem[] = {'D', 'm', 'i', 't', 'r', 'y'};
      constexpr auto sz = sizeof(substr); // size includes '\0'
      static_assert(sizeof(mem) >= sz, "Ill-formed test");
      std::unique_ptr<void, void (*)(void*)> storage{mem, [](void*){ /* dummy deleter */ }};
      const auto d = pgfe::Data::make(std::move(storage), sz, pgfe::Data_format::binary);
      DMITIGR_ASSERT(d->format() == pgfe::Data_format::binary);
      DMITIGR_ASSERT(d->size() == sz);
      DMITIGR_ASSERT(std::strncmp(static_cast<const char*>(d->bytes()), "Dmit", sz - 1) == 0);
    }

    // Data::make(std::string&&, Data_format)
    {
      const char* const name = "Dmitry Igrishin";
      const std::size_t sz = std::strlen(name);
      const auto d = pgfe::Data::make(std::string{name}, pgfe::Data_format::text);
      DMITIGR_ASSERT(d->format() == pgfe::Data_format::text);
      DMITIGR_ASSERT(d->size() == sz);
      DMITIGR_ASSERT(to<std::string_view>(*d) == name);
    }

    // ---------------------------------------------------------------------------
    // Operators
    // ---------------------------------------------------------------------------

    // <, <=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(*lhs < *rhs);              \
      DMITIGR_ASSERT(*lhs <= *rhs);             \
      DMITIGR_ASSERT(!(*lhs == *rhs));          \
      DMITIGR_ASSERT(*lhs != *rhs);             \
      DMITIGR_ASSERT(!(*lhs > *rhs));           \
      DMITIGR_ASSERT(!(*lhs >= *rhs))

      auto lhs = pgfe::Data::make("dima");
      auto rhs = pgfe::Data::make("olga");
      ASSERTMENTS;
      rhs = pgfe::Data::make("olgaolga");
      ASSERTMENTS;
#undef ASSERTMENTS
    }

    // ==, <=, >=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(!(*lhs < *rhs));           \
      DMITIGR_ASSERT(*lhs <= *rhs);             \
      DMITIGR_ASSERT(*lhs == *rhs);             \
      DMITIGR_ASSERT(!(*lhs != *rhs));          \
      DMITIGR_ASSERT(!(*lhs > *rhs));           \
      DMITIGR_ASSERT(*lhs >= *rhs)

      auto lhs = pgfe::Data::make("dima");
      auto rhs = pgfe::Data::make("dima");
      ASSERTMENTS;
      lhs = pgfe::Data::make("");
      rhs = pgfe::Data::make("");
      ASSERTMENTS;
#undef ASSERTMENTS
    }

    // >, >=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(!(*lhs < *rhs));           \
      DMITIGR_ASSERT(!(*lhs <= *rhs));          \
      DMITIGR_ASSERT(!(*lhs == *rhs));          \
      DMITIGR_ASSERT(*lhs != *rhs);             \
      DMITIGR_ASSERT(*lhs > *rhs);              \
      DMITIGR_ASSERT(*lhs >= *rhs)

      auto lhs = pgfe::Data::make("olga");
      auto rhs = pgfe::Data::make("dima");
      ASSERTMENTS;
      lhs = pgfe::Data::make("olgaolga");
      ASSERTMENTS;
#undef ASSERTMENTS
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
