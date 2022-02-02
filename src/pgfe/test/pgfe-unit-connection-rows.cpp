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
#include "../../pgfe.hpp"

struct Person final {
  int id;
  std::string name;
  unsigned int age;
};

void print(const Person& p)
{
  std::cout << "{\n";
  std::cout << "  id: " << p.id << "\n";
  std::cout << "  name: " << p.name << "\n";
  std::cout << "  age: " << p.age << "\n";
  std::cout << "}\n";
}

namespace dmitigr::pgfe {

template<> struct Conversions<Person> final {
  static Person to_type(Row&& r)
  {
    DMITIGR_ASSERT(r);
    Person p;
    p.id   = to<int>(r["id"]);
    p.name = to<std::string>(r["name"]);
    p.age  = to<unsigned int>(r["age"]);
    return p;
  }
};

} // namespace dmitigr::pgfe

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  // Connecting.
  const auto conn = pgfe::test::make_connection();
  conn->connect();

  // Preparing to test -- creating and filling the test table.
  conn->execute(
    "create temp table person("
    "id serial not null primary key,"
    "name text not null,"
    "age integer not null)");
  conn->execute("insert into person (name, age) values('Alla', 30),('Bella', 33)");

  // Test 1a.
  {
    std::cout << "From rows created on the server side:" << std::endl;
    std::vector<Person> persons;
    conn->execute([&persons](auto&& row)
    {
      persons.emplace_back(pgfe::to<Person>(std::move(row)));
    }, "select * from person");
    DMITIGR_ASSERT(persons.size() == 2);
    print(persons[0]);
    print(persons[1]);
  }

  // Test 1b.
  {
    conn->execute("begin");
    conn->execute(
      "create or replace function all_persons()"
      " returns setof person language sql as $function$"
      " select * from person;"
      " $function$;");

    std::cout << "From rows created on the server side by function all_persons:\n";
    std::vector<Person> persons;
    conn->invoke([&persons](auto&& row)
    {
      persons.emplace_back(pgfe::to<Person>(std::move(row)));
    }, "all_persons");
    DMITIGR_ASSERT(persons.size() == 2);
    print(persons[0]);
    print(persons[1]);

    conn->execute("rollback");
  }

  // Test 1c.
  {
    using pgfe::a;

    conn->execute("begin");

    conn->execute(
      "create or replace function persons_by_name(fname text)"
      " returns setof person language sql as $function$"
      " select * from person where name ~ fname;"
      " $function$;");

    std::cout << "From rows created on the server side by function persons_by_name:" << std::endl;
    std::vector<Person> persons;
    conn->invoke([&persons](auto&& row)
    {
      persons.emplace_back(pgfe::to<Person>(std::move(row)));
    }, "persons_by_name", a{"fname", "^B"});
    DMITIGR_ASSERT(persons.size() == 1);
    for (const auto& person : persons)
      print(person);

    conn->execute("rollback");
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
