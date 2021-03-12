// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include "dmitigr/pgfe.hpp"

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

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
    ASSERT(r);
    Person p;
    p.id   = to<int>(r["id"]);
    p.name = to<std::string>(r["name"]);
    p.age  = to<unsigned int>(r["age"]);
    return p;
  }
};

} // namespace dmitigr::pgfe

int main(int, char* argv[])
try {
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
    ASSERT(persons.size() == 2);
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
    ASSERT(persons.size() == 2);
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
    ASSERT(persons.size() == 1);
    for (const auto& person : persons)
      print(person);

    conn->execute("rollback");
  }
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
