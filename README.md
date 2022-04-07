# [PostgreSQL] C++ driver

`dmitigr::pgfe` (PostGres FrontEnd, hereinafter referred to as Pgfe) - is a
[PostgreSQL] C++ driver. The development is focused on easines and robustness of
use with performance in mind.

## Upcoming release 2.0

I'm currently working on Pgfe 2.0. The current stage is beta!

## Hello, World

```cpp
#include <dmitigr/pgfe/pgfe.hpp>

#include <cstdio>

namespace pgfe = dmitigr::pgfe;

int main() try {
  // Making the connection.
  pgfe::Connection conn{pgfe::Connection_options{}
    .set(pgfe::Communication_mode::net)
    .set_hostname("localhost")
    .set_database("pgfe_test")
    .set_username("pgfe_test")
    .set_password("pgfe_test")};

  // Connecting.
  conn.connect();

  // Using Pgfe's helpers.
  using pgfe::a;  // for named arguments
  using pgfe::to; // for data conversions

  // Executing statement with positional parameters.
  conn.execute([](auto&& r)
  {
    std::printf("Number %i\n", to<int>(r.data()));
  }, "select generate_series($1::int, $2::int)", 1, 3);

  // Execute statement with named parameters.
  conn.execute([](auto&& r)
  {
    std::printf("Range [%i, %i]\n", to<int>(r["b"]), to<int>(r["e"]));
  },"select :begin b, :end e", a{"end", 1}, a{"begin", 0});

  // Prepare and execute the statement.
  auto ps = conn.prepare("select $1::int i");
  for (int i{}; i < 3; ++i)
    ps.execute([](auto&& r){std::printf("%i\n", to<int>(r["i"]));}, i);

  // Invoking the function.
  conn.invoke([](auto&& r)
  {
    std::printf("cos(%f) = %f\n", .5f, to<float>(r.data()));
  }, "cos", .5f);

  // Provoking the syntax error.
  conn.execute("provoke syntax error");
 } catch (const pgfe::Server_exception& e) {
  assert(e.error().condition() == pgfe::Server_errc::c42_syntax_error);
  std::printf("Error %s is handled as expected.\n", e.error().sqlstate());
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 }
```

## Features

  - fast (negligible overhead compared to libpq);
  - can be used as header-only library;
  - work with database connections (in both blocking and non-blocking IO manner);
  - execute prepared statements (named parameters are supported);
  - conveniently call functions and procedures;
  - conveniently handle errors by either via exceptions or error conditions;
  - conveniently work with [large objects][lob], [COPY][copy] and [pipeline];
  - enum entry for each predefined [SQLSTATE][errcodes];
  - easily convert the data from the client side representation to the server
    side representation and vice versa (conversions of multidimensional
    [PostgreSQL] arrays to/from any combinations of STL containers are supported
    out of the box!);
  - dynamically construct SQL queries;
  - separate SQL and C++ code (e.g., by placing SQL code into a text file);
  - simple and thread-safe connection pool.

## Usage

### Quick usage as header-only library

Copy the contents of the `src` directory to a project directory which is under
an include path of a compiler, for example, `src/3rdparty/dmitigr`.

Create `hello.cpp`:

```C++
#include "dmitigr/pgfe/pgfe.hpp"

int main()
{
  dmitigr::pgfe::Connection conn;
}
```

Compile `hello.cpp`:

```
g++ -std=c++17 -I/usr/local/pgsql/include -L/usr/local/pgsql/lib -lpq -ohello hello.cpp
```

### Quick usage with CMake

Create build directory, configure, build and install (note, if libpq is in a
non-standard location, then the path to it can be specified by using
`-DLIBPQ_PREFIX` as shown below):

```
cd pgfe
mkdir build && cd build
cmake -DLIBPQ_PREFIX=/usr/local/pgsql .. # -DLIBPQ_PREFIX is optional
cmake --build .
sudo cmake --install .
```

Create `hello/hello.cpp`:

```C++
#include "pgfe/pgfe.hpp"

int main()
{
  dmitigr::pgfe::Connection conn;
}
```

Create `hello/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(foo)
find_package(dmitigr_cpplipa REQUIRED COMPONENTS pgfe)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(hello hello.cpp)
target_link_libraries(hello dmitigr_pgfe)
```

Compile `hello/hello.cpp`:

```
mkdir hello/build && cd hello/build
cmake ..
cmake --build .
```

### Advanced usage

For more details please, see [usage section][dmitigr_cpplipa_usage] for hints
how to link Pgfe.

## Quick tutorial

Logically, Pgfe library consists of the following parts:

  - main (client/server communication);
  - data types conversions;
  - errors (exceptions and error conditions);
  - utilities.

The API is defined in the namespace `dmitigr::pgfe`. In this tutorial all the
names are not explicitly qualified by this namespace.

### Connecting to a server

By using class `Connection_options` it's easy to specify the required connection
options:

```cpp
// Example 1. Making connection options.
auto make_options()
{
  return Connection_options{}
    .set(Communication_mode::net)
    .set_net_hostname("localhost")
    .set_database("db")
    .set_username("user")
    .set_password("password");
}
```

By using class `Connection` it's easy to connect to the PostgreSQL server:

```cpp
// Example 2. Making ready-to-use connection.
auto make_connection(const Connection_options& opts = {})
{
  Connection conn{opts};
  conn.connect(); // connect synchronously (in blocking manner)
  return conn; // return the ready-to-use instance
}
```

### Executing SQL commands

Only extended query protocol is used under the hood to execute SQL commands. SQL
commands can be executed and processed either synchronously or in non-blocking IO
maner, i.e. without need of waiting a server response(-s), and thus, without thread
blocking. In the latter case the methods of the class `Connection` with the suffix
`_nio` shall be used.

With Pgfe it's easy to execute single commands:

```cpp
// Example 3. Executing single commands.
void foo(Connection& conn)
{
  conn.execute("begin");
  conn.execute("create temp table num(val integer not null)");
  conn.execute([](auto&& row)
  {
    using dmitigr::pgfe::to;    // see "Data conversions" section for details
    auto val = to<int>(row[0]); // converts the value of num.val to int
    std::cout << val << "\n";   // prints the just inserted integers
  }, "insert into num select generate_series(1,3) returning val");
  conn.execute("rollback");
}
```

Extended query protocol used by Pgfe is based on prepared statements. In Pgfe
prepared statements can be parameterized with either positional or named
parameters. The class `Statement` provides functionality for constructing SQL
statements, providing support for named parameters, as well as functionality for
direct parameters replacement with any SQL statement to generate complex SQL
expressions dynamically.

When using "extended query" protocol in its simplest form unnamed statements are
prepared implicitly:

```cpp
// Example 4. Preparing unnamed statements (`BEGIN`, `SELECT`, `ROLLBACK`) implicitly.
void foo(Connection& conn)
{
  conn.execute("begin");
  conn.execute([](auto&& row)
  {
    using dmitigr::pgfe::to;            // see "Data conversions" section for details
    auto val = to<std::string>(row[0]); // converts the retrieved value to std::string
    std::cout << val << "\n";           // prints "Hi!\n"
  }, "select 'Hi!'");
  conn.execute("rollback");
}
```

It's also easy to use named parameters:

```cpp
// Example 5. Using named parameters in statements.
void foo(Connection& conn)
{
  // Please note, the sequence of the specified named parameters doesn't matter,
  // and that "end" parameter is specified before "begin" parameter.
  conn.execute([](auto&& row)
  {
    std::printf("Range [%i, %i]\n", to<int>(row["b"]), to<int>(row["e"]));
  },"select :begin b, :end e", a{"end", 1}, a{"begin", 0});
}
```

Of course, the statement (named or unnamed) can be prepared explicitly:

```cpp
// Example 6. Preparing the unnamed statement parameterized by named parameters.
void foo(Connection& conn)
{
  conn.prepare("select generate_series(:inf::int, :sup::int) num")
    .bind("inf", 1)
    .bind("sup", 3)
    .execute([](auto&& row)
    {
      // Printing the just generated integers without type conversion
      std::printf("%s\n", row["num"].bytes());
    });
}
```

### Invoking functions and calling procedures

Pgfe provides the convenient API for functions invoking or procedures calling:
methods `Connection::invoke()`, `Connection::invoke_unexpanded()` and
`Connection::call()` accordingly.

To illustrate the API the following function definition is used:

```sql
create function person_info(id integer, name text, age integer)
returns text language sql as
$$
  select format('id=%s name=%s age=%s', id, name, age);
$$;
```

Calling "person_info" by using positional notation:

```cpp
// Example 7. Using positional notation.
void foo(Connection& conn)
{
  conn.invoke("person_info", 1, "Dmitry", 36);
  // ...
}
```

Calling "person_info" by using named notation:

```cpp
// Example 8. Using named notation.
void foo(Connection& conn)
{
  using dmitigr::pgfe::a;
  conn.invoke("person_info", a{"name", "Dmitry"}, a{"age", 36}, a{"id", 1});
  // ...
}
```

Calling "person_info" by using mixed notation:

```cpp
// Example 9. Using mixed notation.
void foo(Connection& conn)
{
  using dmitigr::pgfe::a;
  conn.invoke("person_info", 1, a{"age", 36}, a{"name", "Dmitry"});
  // ...
}
```

### Data conversions

Pgfe provides the support of conversions only for *fundamental and standard C++
types* out of the box. Conversions for special PostgreSQL types such as
[Date/Time Types][datatype-datetime] aren't provided out of the box, since many
implementations of these types are possible at the client side. Instead it's up
to the user to decide what implementation to use. (If such conversions are needed
at all.) For example, the template structure `Conversions` can be easily
specialized to convert the data between PostgreSQL [Date/Time Types][datatype-datetime]
and types from the [Boost.Date_Time][boost_datetime] library.

The abstract class `Data` is designed to provide the interface for:

  - the values of prepared statements' parameters;
  - the data retrieved from a [PostgreSQL] server.

The template structure `Conversions` are used by:

  - method `Prepared_statement::bind(std::size_t, T&&)` to perfrom data conversions
  from objects or type `T` to objects of type `Data`;
  - function `to()` to perform data conversions from objects of type `Data` to objects
  of the specified type `T`.

Pgfe provides the partial specialization of the template structure `Conversions` to
convert from/to [PostgreSQL] arrays (*including multidimensional arrays!*)
representation to **any combination of the STL containers** out of the box! (At the
moment, arrays conversions are only implemented for `Data_format::text` format.) In
general, *any* [PostgreSQL] array can be represented as `Container<Optional<T>>`,
where:

  - `Container` - is a template class of a container such as
    [`std::vector`][std_vector] or [`std::list`][std_list] or [`std::deque`][std_deque];
  - `Optional` - is a template class of an optional value holder such as
    [`std::optional`][std_optional] or [`boost::optional`][boost_optional]. The
    special value like [`std::nullopt`][std_nullopt] represents the SQL `NULL`;
  - `T` - is the type of elements of the array. It can be `Container<Optional<U>>`
    to represent the multidimensional array.

In case when all the elements of the array are not `NULL`, it *can* be represented
as the container with elements of type `T` rather than `Optional<T>`. But in case
when the source array (which comes from the PostgreSQL server) contain at least
one `NULL` element a `Client_exception` will be thrown. Summarizing:

  - the types `Container<Optional<T>>`, `Container<Optional<Container<Optional<T>>>>`,
    `...` can be used to represent N-dimensional arrays of `T` which *may* contain `NULL`
    values;

  - the types `Container<T>`, `Container<Container<T>>`, `...` can be used to represent
    N-dimensional arrays of `T` which *may not* contain `NULL` values.

User-defined data conversions could be implemented by either:

  - overloading the operators `operator<<` and `operator>>` for
    [`std::ostream`][std_ostream] and [`std::istream`][std_istream] respectively;
  - specializing the template structure `Conversions`. (With this approach overheads
    of standard IO streams can be avoided.)

### Response processing

Server responses can be retrieved:

  - implicitly in blocking IO manner by using methods such as `Connection::perform()`,
    `Connection::prepare()`, `Connection::execute()` etc. Some of these methods has
    overloads for passing the callback which is called by Pgfe every time the row
    is retrieved from the server;
  - explicitly in blocking IO manner by using methods such as `Connection::wait_response()`
    and `Connection::wait_response_throw()` etc after the using methods with the
    suffix "_nio";
  - explicitly in non-blocking IO maner by using the methods such as `Connection::read_input()`,
    `Connection::handle_input()`, `Connection::socket_readiness()` etc after the
    using methods with suffix "_nio".

To *initiate* retrieving the *first* response in non-blocking IO manner methods
of the class `Connection` with the suffix `_nio` must be used. Otherwise, Pgfe
will wait for the *first* response and if that response is error, an instance of
type `Server_exception` will be thrown. This object provides access to the object
of type `Error`, which contains all the error details.

Server responses are represented by the classes inherited from `Response`:

  - errors are represented by the class `Error`. Each server error is identifiable
    by a [SQLSTATE][errcodes] condition. In Pgfe *each* such a condition is represented
    by the member of the enumeration `Server_errc`, integrated to the framework for
    reporting errors provided by the standard library in [`<system_error>`][system_error].
    Therefore, working with [SQLSTATEs][errcodes] is as simple and safe as
    with [`std::error_condition`][std_error_condition] and enumerated types, for example:

```cpp
// Example 10. Catching the syntax error.
void foo(Connection& conn)
{
  try {
    conn.perform("provoke syntax error");
  } catch (const Server_exception& e) {
    assert(e.error().condition() == Server_errc::c42_syntax_error);
  }
}
```

  - rows are represented by the class `Row`, for example:

```cpp
// Example 11. Processing the rows.
void foo(Connection& conn)
{
  conn.execute([](auto&& row)
  {
    using dmitigr::pgfe::to;
    auto name = to<std::string>(row["name"]);
    std::printf("%s\n", name.data());
  }, "select name from usr where id = $1", 3); // where id = 3
}
```

  - prepared statements are represented by the class `Prepared_statement`, for
    example:

```cpp
// Example 12. Working with named prepared statement.
void foo(Connection& conn)
{
  // Prepare the named statement
  auto& int_gen = conn.prepare("select generate_series($1::int, $2::int)", "int_gen");

  // Defining the row processor
  auto process = [](auto&& row)
  {
    using dmitigr::pgfe::to;
    auto n = to<int>(row[0]);
    std::printf("%i\n", n);
  };

  // Execute for the first time
  int_gen.bind(1).bind(2).execute(process);
  // Execute for the second time
  int_gen.bind(10).bind(20).execute(process);
}
```

  - operation success indicators are represented by the class `Completion`, for
    example:

```cpp
// Example 13. Using completion info.
void foo(Connection& conn)
{
  auto completion = conn.perform("begin");
  std::printf("%s\n", completion.operation_name()); // prints "BEGIN"
}
```

### Signal handling

Server signals are represented by classes `Notice` and `Notification`, inherited
from class `Signal`. Signals can be handled by using the signal handlers (see
`Connection::set_notice_handler()` and `Connection::set_notification_handler()`).
Notifications can also be handled in non-blocking IO maner, by using the method
`Connection::pop_notification()`.

Signal handlers, being set, called by Pgfe automatically when signals are retrieved.
(Usually it happens upon waiting a response.) If no notification handler is set,
notifications will be queued to the internal storage until popped up by method
`Connection::pop_notification()`. **Be aware, that if notification are not popped
up from the internal storage it may cause memory exhaustion!**

### Dynamic SQL

The standard classes like [`std::string`][std_string] or [`std::ostringstream`][std_ostringstream]
can be used to make SQL strings dynamically. However, in some cases it is more
convenient to use the class `Statement` for this purpose. Consider the following
statement:

```sql
select :expr::int, ':expr';
```

This SQL string has one named parameter `expr` and one string constant
`':expr'`. It's possible to replace the named parameters of the SQL string with
another SQL string by using `Statement::replace_parameter()`, for example:

```cpp
// Example 14. Extending the SQL statement.
void foo()
{
  Statement sql{"select :expr::int, ':expr'"};
  sql.replace_parameter("expr", "sin(:expr1::int), cos(:expr2::int)");
}
```

Now the original statement is modified and has two named parameters:

```sql
select sin(:expr1::int), cos(:expr2::int), ':expr'
```

Note, that the quoted string `:expr` is not affected by the replacement operation.

### Working with SQL code separately of C++ code

This feature is based on the idea to store the SQL code in a separate place,
such as a text file. Consider the following SQL input, which is consists of two
SQL strings with an extra data specified by the [dollar-quoted][dollar-quoting]
string constants in the related comments:

```sql
-- This is query 1
--
-- $id$plus-one$id$
select :n::int + 1, ';'; -- note, the semicolons in quotes are allowed!

/* This is query 2
 *
 * $id$minus-one$id$
 */
select :n::int - 1
```

These SQL strings can be easily accessed by using class `Statement_vector`:

```cpp
// Example 15. Parsing file with SQL statements.

std::string read_file(const std::filesystem::path& path); // defined somewhere

void foo()
{
  const auto input = read_file("bunch.sql");
  Statement_vector bunch{input};
  auto* minus_one = bunch.find("id", "minus-one"); // select :n::int - 1
  auto*  plus_one = bunch.find("id",  "plus-one"); // select :n::int + 1, ';'
  // ...
}
```

### Connection pool

Pgfe provides a simple connection pool implemented in class `Connection_pool`:

```cpp
// Example 16. Using the connection pool.

Connection_options connection_options(); // defined somewhere.

int main()
{
  Connection_pool pool{2, connection_options()};
  pool.connect(); // open 2 connections
  {
    auto conn1 = pool.connection(); // 1st attempt to get the connection from pool
    assert(conn1);  // ok
    conn1.perform("select 1");
    auto conn2 = pool.connection(); // 2nd attempt to get the connection from pool
    assert(conn2);  // ok
    conn2.perform("select 2");
    auto conn3 = pool.connection(); // 3rd attempt to get the connection from pool
    assert(!conn3); // the pool is exhausted
  } // connections are returned back to the pool here
  auto conn = pool.connection();
  assert(conn); // ok
  pool.disconnect(); // done with the pool
}
```

## Exceptions

Pgfe itself may throw:

  - an instance of the type `Client_exception` when error originated on the
  client side;
  - an instance of the type `Server_exception` when error originated on the
  server side upon using of IO blocking API.

## Thread safety

By default, if not explicitly documented, all functions and methods of Pgfe are
*not* thread safe. Thus, in most cases, some of the synchronization mechanisms
(like mutexes) must be used to work with the same object from several threads.

## Dependencies

Pgfe is depends on the [libpq] library.

## CMake options

Since Pgfe is a C++ library subpackage of [Cpplipa][dmitigr_cpplipa], almost
all the [CMake options of Cpplipa][dmitigr_cpplipa_cmake_options] are applicable
to Pgfe.

Please, pay attention to the following:

  - by default, `CMAKE_BUILD_TYPE` is set to `Debug`;
  - by using `Pq_ROOT` it's possible to specify a prefix for both binary and
  headers of the [libpq]. For example, if [PostgreSQL] installed relocatably
  into `/usr/local/pgsql`, the value of `Pq_ROOT` should be set accordingly;
  - when building with Microsoft Visual Studio the value of `CMAKE_BUILD_TYPE`
  doesn't selects the build configuration within the generated build environment.
  The [CMake] command line option `--config` should be used for that purpose.

[dmitigr_cpplipa]: https://github.com/dmitigr/cpplipa
[dmitigr_cpplipa_cmake_options]: https://github.com/dmitigr/cpplipa#cmake-options
[dmitigr_cpplipa_usage]: https://github.com/dmitigr/cpplipa.git#usage
[dmitigr_pgfe]: https://github.com/dmitigr/pgfe.git

[PostgreSQL]: https://www.postgresql.org/
[dollar-quoting]: https://www.postgresql.org/docs/current/static/sql-syntax-lexical.html#SQL-SYNTAX-DOLLAR-QUOTING
[datatype-datetime]: https://www.postgresql.org/docs/current/datatype-datetime.html
[errcodes]: https://www.postgresql.org/docs/current/static/errcodes-appendix.html
[libpq]: https://www.postgresql.org/docs/current/static/libpq.html
[lob]: https://www.postgresql.org/docs/current/static/largeobjects.html
[copy]: https://www.postgresql.org/docs/current/sql-copy.html
[pipeline]: https://www.postgresql.org/docs/current/libpq-pipeline-mode.html

[boost_datetime]: https://www.boost.org/doc/libs/release/libs/date_time/
[boost_optional]: https://www.boost.org/doc/libs/release/libs/optional/

[CMake]: https://cmake.org/
[Doxygen]: http://doxygen.org/

[system_error]: https://en.cppreference.com/w/cpp/header/system_error
[std_deque]: https://en.cppreference.com/w/cpp/container/deque
[std_error_condition]: https://en.cppreference.com/w/cpp/error/error_condition
[std_istream]: https://en.cppreference.com/w/cpp/io/basic_istream
[std_list]: https://en.cppreference.com/w/cpp/container/list
[std_optional]: https://en.cppreference.com/w/cpp/utility/optional
[std_ostringstream]: https://en.cppreference.com/w/cpp/io/basic_ostringstream
[std_ostream]: https://en.cppreference.com/w/cpp/io/basic_ostream
[std_nullopt]: https://en.cppreference.com/w/cpp/utility/optional/nullopt
[std_string]: https://en.cppreference.com/w/cpp/string/basic_string
[std_vector]: https://en.cppreference.com/w/cpp/container/vector
