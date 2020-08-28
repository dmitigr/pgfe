// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include <dmitigr/pgfe/data.hpp>
#include <dmitigr/pgfe/exceptions.hpp>
#include <dmitigr/testo.hpp>

#include <cstring>
#include <string>
#include <vector>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    // Data::make(std::string_view)
    {
      const std::size_t sz = std::strlen("Dmitry Igrishin");
      const auto d = pgfe::Data::make("Dmitry Igrishin");
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(std::strcmp(d->bytes(), "Dmitry Igrishin") == 0);
    }

    // Data::make(std::string_view);
    {
      const std::size_t sz = std::strlen("Dmitry");
      const auto d = pgfe::Data::make(std::string_view{"Dmitry Igrishin", sz}, pgfe::Data_format::binary);
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == sz);
      ASSERT(std::strncmp(d->bytes(), "Dmitry", sz) == 0);
    }

    // Data::make(std::unique_ptr<void, void (*)(void*)>&&, std::size_t, Data_format)
    {
      char substr[] = {'D', 'm', 'i', 't', '\0'};
      char mem[] = {'D', 'm', 'i', 't', 'r', 'y'};
      constexpr auto sz = sizeof(substr); // size includes '\0'
      static_assert(sizeof (mem) >= sz, "Ill-formed test");
      std::unique_ptr<void, void (*)(void*)> storage{mem, [](void*){ /* dummy deleter */ }};
      const auto d = pgfe::Data::make(std::move(storage), sz, pgfe::Data_format::binary);
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == sz);
      ASSERT(std::strncmp(d->bytes(), "Dmit", sz - 1) == 0);
    }

    // Data::make(std::string&&, Data_format)
    {
      const char* const name = "Dmitry Igrishin";
      const std::size_t sz = std::strlen(name);
      const auto d = pgfe::Data::make(std::string(name), pgfe::Data_format::text);
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(std::strcmp(d->bytes(), name) == 0);
    }

    // Data::make(const std::string&, Data_format)
    {
      const std::string name{"Dmitry Igrishin"};
      const std::size_t sz = name.size();
      const auto d = pgfe::Data::make(name, pgfe::Data_format::text);
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(d->bytes() == name);
    }
  } catch (std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
