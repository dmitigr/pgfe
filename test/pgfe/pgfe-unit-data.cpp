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
