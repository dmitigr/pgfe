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
