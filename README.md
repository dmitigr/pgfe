# C++ client API to PostgreSQL {#mainpage}

Dmitigr Pgfe (PostGres FrontEnd, hereinafter referred to as Pgfe) - is a C++
client API to [PostgreSQL] servers. The development is focused on easines and
robustness of use. At the same time, everything possible is being done to ensure
that the performance is at its best. Pgfe is a part of the [Dmitigr Cefeika][dmitigr_cefeika]
project, but also available as a standalone library [here][dmitigr_pgfe].

## Upcoming release 2.0

**ATTENTION, API breaking changes starting from commit [62ceba3][v20alpha1]!**

I'm currently working on Pgfe 2.0. The current stage is early alpha. Nonetheless,
I recommend to switch to the new API despite the fact that it's still a subject
to change while work on release 2.0 is in progress. (Although I don't think the
changes will be significant.) Efforts will be made to make the API of Pgfe 2.0
as stable as possible.

## Documentation

The [Doxygen]-generated documentation is located [here][dmitigr_pgfe_doc]. There
is [overview class diagram][dmitigr_pgfe_doc_diagram].

## Hello, World

```cpp
#include <dmitigr/pgfe.hpp>
#include <cstdio>

namespace pgfe = dmitigr::pgfe;

int main() try {
  // Making the connection.
  pgfe::Connection conn{pgfe::Connection_options{pgfe::Communication_mode::net}
    .net_hostname("localhost").database("pgfe_test")
    .username("pgfe_test").password("pgfe_test")};

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
  auto* const ps = conn.prepare("select $1::int i");
  for (int i = 0; i < 3; ++i)
    ps->execute([](auto&& r){ std::printf("%i\n", to<int>(r["i"])); }, i);

  // Invoking the function.
  conn.invoke([](auto&& r)
  {
    std::printf("cos(%f) = %f\n", .5f, to<float>(r.data()));
  }, "cos", .5f);

  // Provoking the syntax error.
  conn.execute("provoke syntax error");
 } catch (const pgfe::c42_Syntax_error& e) {
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
  - conveniently handle errors by either via exceptions or error codes;
  - conveniently work with [large objects][lob];
  - exception class and enum entry for each predefined [SQLSTATE][errcodes] code;
  - easily convert the data from the client side representation to the server
    side representation and vice versa (conversions of multidimensional
    [PostgreSQL] arrays to/from any combinations of STL containers are supported
    out of the box!);
  - dynamically construct SQL queries;
  - separate SQL and C++ code (e.g., by placing SQL code into a text file);
  - simple and thread-safe connection pool.

## Usage

Please, see [Cefeika Usage][dmitigr_cefeika_usage] section for hints how to link Pgfe.

## Quick tutorial

Logically, Pgfe library consists of the following parts:

  - main (client/server communication);
  - data types conversions;
  - errors (exceptions and error codes);
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
  return Connection_options{Communication_mode::net}
    .net_hostname("localhost")
    .database("db")
    .username("user")
    .password("password");
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

Since [v20alpha2] only extended query protocol is used under the hood to execute SQL
commands. SQL commands can be executed and processed either synchronously or in
non-blocking IO maner, i.e. without need of waiting a server response(-s), and thus,
without thread blocking. In the latter case the methods of the class `Connection` with
the suffix `_nio` shall be used.

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

Extended query protocol used by Pgfe is based on prepared statements. In Pgfe prepared
statements can be parameterized with either positional or named parameters. The
class `Sql_string` provides functionality for constructing SQL statements,
providing support for named parameters, as well as functionality for direct
parameters replacement with any SQL statement to generate complex SQL expressions
dynamically.

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
    ->bind("inf", 1)
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
one `NULL` element a runtime exception will be thrown. Summarizing:

  - the types `Container<Optional<T>>`, `Container<Optional<Container<Optional<T>>>>`, `...`
    can be used to represent N-dimensional arrays of `T` which *may* contain `NULL` values;

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
    by a [SQLSTATE][errcodes] code. In Pgfe *each* such a code is represented by
    the member of the enumeration `Server_errc`, integrated to the framework for
    reporting errors provided by the standard library in [`<system_error>`][system_error].
    Therefore, working with [SQLSTATE][errcodes] codes is as simple and safe as
    with [`std::error_code`][std_error_code] and enumerated types, for example:

```cpp
// Example 10. Catching the syntax error.
void foo(Connection& conn)
{
  try {
    conn.perform("provoke syntax error");
  } catch (const Server_exception& e) {
    assert(e.error().code() == Server_errc::c42_syntax_error);
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
  auto* int_gen = conn.prepare("select generate_series($1::int, $2::int)", "int_gen");

  // Defining the row processor
  auto process = [](auto&& row)
  {
    using dmitigr::pgfe::to;
    auto n = to<int>(row[0]);
    std::printf("%i\n", n);
  };

  // Execute for the first time
  int_gen->bind(1).bind(2).execute(process);
  // Execute for the second time
  int_gen->bind(10).bind(20).execute(process);
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

Server signals are represented by classes, inherited from `Signal`:

  - notices are represented by the class `Notice`;
  - notifications are represented by the class `Notification`.

Signals can be handled:

  - by using the signal handlers (see `Connection::set_notice_handler()`,
    `Connection::set_notification_handler()`);

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
convenient to use the class `Sql_string` for this purpose. Consider the following
statement:

```sql
select :expr::int, ':expr';
```

This SQL string has one named parameter `expr` and one string constant
`':expr'`. It's possible to replace the named parameters of the SQL string with
another SQL string by using `Sql_string::replace_parameter()`, for example:

```cpp
// Example 14. Extending the SQL statement.
void foo()
{
  Sql_string sql{"select :expr::int, ':expr'"};
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

These SQL strings can be easily accessed by using class `Sql_vector`:

```cpp
// Example 15. Parsing file with SQL statements.

std::string read_file(const std::filesystem::path& path); // defined somewhere

void foo()
{
  const auto input = read_file("bunch.sql");
  Sql_vector bunch{input};
  auto* minus_one = bunch.find("id", "minus-one"); // select :n::int - 1
  auto*  plus_one = bunch.find("id",  "plus-one"); // select :n::int + 1, ';'
  // ...
}
```

### Connection pool

Pgfe provides a simple connection pool implemented in class `Connection_pool`:

```cpp
// Example 16. Using the connection pool.

inline std::unique_ptr<Connection_pool> pool;
Connection_options connection_options(); // defined somewhere.

int main()
{
  Connection_pool pool{2, connection_options()};
  pool.connect(); // opening up 2 connections
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

Pgfe may throw:

  - an instance of the type [`std::logic_error`][std_logic_error] when using
    `assert()` is not good enough (for example, when checking the assertion must
    be done regardless of value of `NDEBUG` upon build);
  - an instance of the types [`std::runtime_error`][std_runtime_error] or
    `Client_exception` when some kind of runtime error occured on the client side;
  - an instance of the type `Server_exception` when some error occured on the
    server side and IO blocking API is in use.

## Thread safety

By default, if not explicitly documented, all functions and methods of Pgfe are
*not* thread safe. Thus, in most cases, some of the synchronization mechanisms
(like mutexes) must be used to work with the same object from several threads.

## Dependencies

Pgfe is depends on the [libpq] library.

## CMake options

The table below (one may need to use horizontal scrolling for full view) contains
variables which can be passed to [CMake] for customization of the Pgfe library.

|CMake variable|Possible values|Default on Unix|Default on Windows|
|:-------------|:--------------|:--------------|:-----------------|
|**Defaults**||||
|DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE|uds \| net|uds|net|
|DMITIGR_PGFE_CONNECTION_UDS_DIRECTORY|*an absolute path*|/tmp|*unavailable*|
|DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME|*a string*|*not set*|*unavailable*|
|DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED|On \| Off|Off|Off|
|DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE|*non-negative number*|*null (system default)*|*null (system default)*|
|DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL|*non-negative number*|*null (system default)*|*null (system default)*|
|DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT|*non-negative number*|*null (system default)*|*null (system default)*|
|DMITIGR_PGFE_CONNECTION_NET_ADDRESS|*IPv4 or IPv6 address*|127.0.0.1|127.0.0.1|
|DMITIGR_PGFE_CONNECTION_NET_HOSTNAME|*a string*|localhost|localhost|
|DMITIGR_PGFE_CONNECTION_PORT|*a number*|5432|5432|
|DMITIGR_PGFE_CONNECTION_USERNAME|*a string*|postgres|postgres|
|DMITIGR_PGFE_CONNECTION_DATABASE|*a string*|postgres|postgres|
|DMITIGR_PGFE_CONNECTION_PASSWORD|*a string*|""|""|
|DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME|*a string*|*null (not used)*|*null (not used)*|
|DMITIGR_PGFE_CONNECTION_SSL_ENABLED|On \| Off|Off|Off|
|DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOSTNAME_VERIFICATION_ENABLED|On \| Off|Off|Off|
|DMITIGR_PGFE_CONNECTION_SSL_COMPRESSION_ENABLED|On \| Off|Off|Off|
|DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE|*an absolute path*|*null (libpq's default)*|*null (libpq's default)*|
|DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE|*an absolute path*|*null (libpq's default)*|*null (libpq's default)*|
|DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE|*an absolute path*|*null (libpq's default)*|*null (libpq's default)*|
|DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE|*an absolute path*|*null (libpq's default)*|*null (libpq's default)*|

## Copyright

Copyright (C) [Dmitry Igrishin][dmitigr_mail]

[dmitigr_mail]: mailto:dmitigr@gmail.com
[dmitigr_cefeika]: https://github.com/dmitigr/cefeika.git
[dmitigr_cefeika_usage]: https://github.com/dmitigr/cefeika.git#usage
[dmitigr_pgfe]: https://github.com/dmitigr/pgfe.git
[dmitigr_pgfe_doc]: http://dmitigr.ru/en/projects/cefeika/pgfe/doc/
[dmitigr_pgfe_doc_diagram]: http://dmitigr.ru/en/projects/cefeika/pgfe/doc/dmitigr_pgfe_overview.violet.html

[v20alpha1]: https://github.com/dmitigr/pgfe/commit/62ceba3e4e1285178d223fdadaf6ca87c6d514d9
[v20alpha2]: https://github.com/dmitigr/pgfe/commit/c69d3625175d30515319efc738dca8e7d5e9af2a

[PostgreSQL]: https://www.postgresql.org/
[dollar-quoting]: https://www.postgresql.org/docs/current/static/sql-syntax-lexical.html#SQL-SYNTAX-DOLLAR-QUOTING
[datatype-datetime]: https://www.postgresql.org/docs/current/datatype-datetime.html
[errcodes]: https://www.postgresql.org/docs/current/static/errcodes-appendix.html
[libpq]: https://www.postgresql.org/docs/current/static/libpq.html
[lob]: https://www.postgresql.org/docs/current/static/largeobjects.html

[boost_datetime]: https://www.boost.org/doc/libs/release/libs/date_time/
[boost_optional]: https://www.boost.org/doc/libs/release/libs/optional/

[CMake]: https://cmake.org/
[Doxygen]: http://doxygen.org/

[system_error]: https://en.cppreference.com/w/cpp/header/system_error
[std_deque]: https://en.cppreference.com/w/cpp/container/deque
[std_error_code]: https://en.cppreference.com/w/cpp/error/error_code
[std_istream]: https://en.cppreference.com/w/cpp/io/basic_istream
[std_list]: https://en.cppreference.com/w/cpp/container/list
[std_logic_error]: https://en.cppreference.com/w/cpp/error/logic_error
[std_optional]: https://en.cppreference.com/w/cpp/utility/optional
[std_ostringstream]: https://en.cppreference.com/w/cpp/io/basic_ostringstream
[std_ostream]: https://en.cppreference.com/w/cpp/io/basic_ostream
[std_nullopt]: https://en.cppreference.com/w/cpp/utility/optional/nullopt
[std_runtime_error]: https://en.cppreference.com/w/cpp/error/runtime_error
[std_string]: https://en.cppreference.com/w/cpp/string/basic_string
[std_vector]: https://en.cppreference.com/w/cpp/container/vector
