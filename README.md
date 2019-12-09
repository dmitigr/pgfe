Client API for PostgreSQL in C++ {#mainpage}
============================================

Dmitigr Pgfe (PostGres FrontEnd, hereinafter referred to as Pgfe) - is a client
API to [PostgreSQL] servers written in C++. The development is focused on
easines and robustness of use. At the same time, everything possible is being
done to ensure that the performance is at its best. Pgfe is a part of the
[Dmitigr Cefeika][dmitigr_cefeika] project, but also available as a standalone
project [here][dmitigr_pgfe].

**ATTENTION, this software is "beta" quality, and the API is a subject to change!**

Documentation
=============

The [Doxygen]-generated documentation is located [here][dmitigr_pgfe_doc]. There
is [overview class diagram][dmitigr_pgfe_doc_diagram].

Hello, World
============

```cpp
#include <dmitigr/pgfe.hpp>
#include <iostream>

int main()
{
  namespace pgfe = dmitigr::pgfe;
  try {
    const auto conn = pgfe::Connection_options::make(pgfe::Communication_mode::net)->
      set_net_hostname("localhost")->
      set_database("pgfe_test")->
      set_username("pgfe_test")->
      set_password("pgfe_test")->
      make_connection();

    conn->connect();
    conn->execute("SELECT generate_series($1::int, $2::int) AS natural", 1, 3);
    conn->for_each([](const auto* const row) {
      std::cout << pgfe::to<int>(row->data("natural")) << "\n";
    });
    std::cout << "The " << conn->completion()->operation_name() << " query is done.\n";

    // As a sample of error handling let's provoke syntax error and handle it away.
    try {
      conn->perform("PROVOKE SYNTAX ERROR");
    } catch (const pgfe::Server_exception& e) {
      if (e.error()->code() == pgfe::Server_errc::c42_syntax_error)
        std::cout << "Error " << e.error()->sqlstate() << " is handled as expected.\n";
      else
        throw;
    }
  } catch (const std::exception& e) {
    std::cerr << "Oops: " << e.what() << std::endl;
    return 1;
  }
}
```

Features
========

  - work with database connections (in both blocking and non-blocking IO manner);
  - execute prepared statements (named parameters are supported);
  - conveniently call functions and procedures;
  - deal with [SQLSTATE][errcodes] codes as simple as with enums;
  - easily convert the data from the client side representation to the server
    side representation and vice versa (conversions of multidimensional
    [PostgreSQL] arrays to/from any combinations of STL containers are supported
    out of the box!);
  - dynamically construct SQL queries;
  - separate SQL and C++ code (e.g., by placing SQL code into a text file).

Features of the future
----------------------

  - exception class for each [SQLSTATE][errcodes] code;
  - [Large Objects][lob] via IO streams of the Standard C++ library;
  - the COPY command;
  - conversions for `dmitigr::pgfe::Composite` data type;
  - yet more convenient work with arrays of variable dimensions at runtime.

Usage
=====

Please, see [Cefeika Usage][dmitigr_cefeika_usage] section for hints how to
link the library to a project.

Tutorial
========

Logically, Pgfe library consists of the following parts:

  - main (client/server communication);
  - large objects (feature of the future, see the above TODO-list);
  - data types conversions;
  - errors (exceptions and error codes);
  - utilities.

Connecting to a server
----------------------

Class `dmitigr::pgfe::Connection` is a central abstraction of the Pgfe library.
By using methods of this class it's possible to:

  - send requests to a server;
  - receive responses from a server (see `dmitigr::pgfe::Response`);
  - receive signals from a server (see `dmitigr::pgfe::Signal`);
  - perform other operations that depend on a server data (such as
    `dmitigr::pgfe::Connection::to_quoted_literal()`).

To make an instance of the class `dmitigr::pgfe::Connection`, the instance of
the class `dmitigr::pgfe::Connection_options` is required. A copy of this
instance is always *read-only* accessible via
`dmitigr::pgfe::Connection::options()`.

Example 1. Creation of the connection with the customized options:

```cpp
std::unique_ptr<dmitigr::pgfe::Connection> create_customized_connection()
{
  return pgfe::Connection_options::make(Communication_mode::net)->
    set_net_hostname("localhost")->
    set_database("db")->
    set_username("user")->
    set_password("password")->
    make_connection();
}
```

Example 2. Creation of the connection with the default options:

```cpp
std::unique_ptr<dmitigr::pgfe::Connection> create_default_connection_1()
{
  const auto opts = pgfe::Connection_options::make();
  return pgfe::Connection::make(opts.get());
}
```

Example 3. Creation of the connection with the default options:

```cpp
std::unique_ptr<dmitigr::pgfe::Connection> create_default_connection_2()
{
  return pgfe::Connection::make();
}
```

After creation of an object of type `dmitigr::pgfe::Connection` there are two
ways to connect available:

  1. synchronously by using `dmitigr::pgfe::Connection::connect()`;
  2. asynchronously by using `dmitigr::pgfe::Connection::connect_async()`.

Executing commands
------------------

SQL commands can be executed through either of two ways:

  1. by using "simple query" protocol (which implies parsing and executing a
     query by a server on each request) with `dmitigr::pgfe::Connection::perform()`;
  2. by using "extended query" protocol (which implies using of parameterizable
     prepared statements):
     + by explicitly preparing a statement with
       `dmitigr::pgfe::Connection::prepare_statement()` and executing it with
       `dmitigr::pgfe::Prepared_statement::execute()`;
     + by implicitly preparing and executing an unnamed prepared statement with
       `dmitigr::pgfe::Connection::execute()`.

Commands can be executed and processed asynchronously, i.e. without need of
waiting a server response(-s), and thus, without thread blocking. For this
purpose the methods of the class `dmitigr::pgfe::Connection` with the suffix
`_async` shall be used, such as `dmitigr::pgfe::Connection::perform_async()`
or `dmitigr::pgfe::Connection::prepare_statement_async()`.

Prepared statements can be parameterized with either positional or named
parameters. In order to use the named parameters, a SQL string must be
preparsed by Pgfe. Preparsed SQL strings are represented by the class
`dmitigr::pgfe::Sql_string`. Unparameterized prepared statements, or prepared
statements parameterized by only positional parameters *does not* require to be
preparsed, and thus, there is no need to create an instance of
`dmitigr::pgfe::Sql_string` in such cases and [`std::string`][std_string]
can be used instead when performance is critical.

To set a value of a prepared statement's parameter it should be converted to an
object of the class `dmitigr::pgfe::Data`. For convenience, there is the templated
method `dmitigr::pgfe::Prepared_statement::set_parameter(std::size_t, T&&)` which
do such a conversion by using one of the specialization of the template structure
`dmitigr::pgfe::Conversions`.

Example 1. Simple querying.

```cpp
void simple_query(dmitigr::pgfe::Connection* const conn)
{
  conn->perform("SELECT generate_series(1, 3) AS num");
}
```

Example 2. Implicit execution of the unnamed prepared statement.

```cpp
void implicit_prepare_and_execute(dmitigr::pgfe::Connection* const conn)
{
  conn->execute("SELECT generate_series($1::int, $2::int) AS num", 1, 3);
}
```

Example 3. Explicit execution of the named prepared statement with named
parameters.

```cpp
void explicit_prepare_and_execute(const std::string& name,
  dmitigr::pgfe::Connection* const conn)
{
  using dmitigr::pgfe::Sql_string;
  static const auto sql = Sql_string::make(
    "SELECT generate_series(:infinum::int, :supremum::int) AS num");
  auto ps = conn->prepare_statement(sql.get(), name);
  ps->set_parameter("infinum",  1);
  ps->set_parameter("supremum", 3);
  ps->execute();
}
```

Invoking functions and calling procedures
-----------------------------------------

In order to invoke a function the methods dmitigr::pgfe::Connection::invoke()
and dmitigr::pgfe::Connection::invoke_unexpanded() can be used. Procedures can
be called by using the method dmitigr::pgfe::Connection::call(). All of these
methods have the same signatures.

To illustrate the API the following function definition is used:

```sql
CREATE FUNCTION person_info(id integer, name text, age integer)
RETURNS text LANGUAGE SQL AS
$$
  SELECT format('id=%s name=%s age=%s', id, name, age);
$$;
```

Example 1. Using Positional Notation.

```cpp
void foo(dmitigr::pgfe::Connection* const conn)
{
  conn->invoke("person_info", 1, "Dmitry", 36);
  // ...
}
```

Example 2. Using Named Notation.

```cpp
void foo(dmitigr::pgfe::Connection* const conn)
{
  using dmitigr::pgfe::_;
  conn->invoke("person_info", _{"name", "Dmitry"}, _{"age", 36}, _{"id", 1});
  // ...
}
```

Example 3. Using Mixed Notation.

```cpp
void foo(dmitigr::pgfe::Connection* const conn)
{
  using dmitigr::pgfe::_;
  conn->invoke("person_info", 1, _{"age", 36}, _{"name", "Dmitry"});
  // ...
}
```

Responses handling
------------------

Server responses are represented by the classes, inherited from
`dmitigr::pgfe::Response`:

  - responses that are server errors are represented by the class
    `dmitigr::pgfe::Error`. Each server error is identifiable by a
    [SQLSTATE][errcodes] code. In Pgfe *each* such a code is represented by
    the member of the enum class `dmitigr::pgfe::Server_errc`, integrated in
    framework for reporting errors provided by the standard library in
    [`<system_error>`][system_error]. Therefore, working with
    [SQLSTATE][errcodes] codes is as simple and safe as with
    [`std::error_code`][std_error_code] and enumerated types! For example:

```cpp
void handle_error_example(dmitigr::pgfe::Connection* const conn)
{
  try {
    conn->perform("PROVOKE SYNTAX ERROR");
  } catch (const dmitigr::pgfe::Server_exception& e) {
    assert(e.error()->code() == dmitigr::pgfe::Server_errc::c42_syntax_error);
  }
}
```

  - responses that are rows are represented by the class `dmitigr::pgfe::Row`.
    Objects of this class can be accessed by using `dmitigr::pgfe::Connection::row()`
    and/or `dmitigr::pgfe::Connection::release_row()`. However, it is best to use
    the method `dmitigr::pgfe::Connection::for_each()` for rows processing.
    **Be aware, that before executing the subsequent operations, all of the rows
    must be processed!**

  - responses that are prepared statements are represented by the class
    `dmitigr::pgfe::Prepared_statement`. Prepared statements are accessible
    via the method `dmitigr::pgfe::Connection::prepared_statement()`.

  - responses that indicates success of operations are represented by the class
    `dmitigr::pgfe::Completion`. Such responses can be accessed by calling
    `dmitigr::pgfe::Connection::completion()` and/or
    `dmitigr::pgfe::Connection::release_completion()`.
    Alternatively, to process completion responses the method
    `dmitigr::pgfe::Connection::complete()` can be used.

To *initiate* asynchronous (i.e. without blocking the thread) retrieving of the
*first* response methods of the class `dmitigr::pgfe::Connection` with the
suffix `_async` must be used. Otherwise, Pgfe will wait for the *first* response
and if that response is `dmitigr::pgfe::Error`, an object of type
`dmitigr::pgfe::Server_exception` will be thrown as exception. This object
provides access to the object of type `dmitigr::pgfe::Error`, which contains
the error details.

Server responses can be retrieved:

  - synchronously by using the methods such as
    `dmitigr::pgfe::Connection::wait_response()` and
    `dmitigr::pgfe::Connection::wait_last_response()`;
  - asynchronously by using the methods such as
    `dmitigr::pgfe::Connection::collect_server_messages()` and
    `dmitigr::pgfe::Connection::socket_readiness()`.

Data type conversions
---------------------

Pgfe ships with support of conversions for *fundamental and standard C++ types*.
Conversions for special PostgreSQL types such as [Date/Time Types][datatype-datetime]
aren't provided out of the box, since many implementations of these types are
possible at the client side. Instead it's up to the user to decide what
implementation to use. (If such conversions are needed at all.) For example, the
template structure `dmitigr::pgfe::Conversions` can be easily specialized to perform
conversions between PostgreSQL [Date/Time Types][datatype-datetime] and types from
the [Boost.Date_Time][boost_datetime] library.

The class `dmitigr::pgfe::Data` is designed to store:

  - the values of prepared statements' parameters;
  - the data retrieved from a [PostgreSQL] server.

The template structure `dmitigr::pgfe::Conversions` are used by:

  - `dmitigr::pgfe::Prepared_statement::set_parameter(std::size_t, T&&)` to perfrom
    data conversions from objects or type `T` to objects of type `dmitigr::pgfe::Data`;
  - `dmitigr::pgfe::to()` to perform data conversions from objects of type
    `dmitigr::pgfe::Data` to objects of the specified type `T`.

There is the partial specialization of the template structure
`dmitigr::pgfe::Conversions` to perform conversions from/to [PostgreSQL] arrays
(*including multidimensional arrays!*) representation to *any* combination of
the STL containers! (At the moment, arrays conversions are only implemented for
`dmitigr::pgfe::Data_format::text` format.) In general, *any* [PostgreSQL] array
can be represented as `Container<Optional<T>>`, where:

  - `Container` - is a template class of a container such as
    [`std::vector`][std_vector] or [`std::list`][std_list] or
    [`std::deque`][std_deque];
  - `Optional` - is a template class of an optional value holder such as
    [`std::optional`][std_optional] or [`boost::optional`][boost_optional]. The
    special value like [`std::nullopt`][std_nullopt] represents the SQL `NULL`;
  - `T` - is the type of elements of the array. It can be `Container<Optional<U>>`
    to represent the multidimensional array.

In case when all of the array elements are non-NULL, it *can* be represented as
the container with elements of type `T` rather than `Optional<T>`. But in case
when the source array (which comes from the PostgreSQL server) contain
at least one NULL element a runtime exception will be thrown. Summarizing:

  - the types `Container<Optional<T>>`,
    `Container<Optional<Container<Optional<T>>>>`, `...`
    can be used to represent N-dimensional arrays of `T`
    which *can* contain NULL values;

  - the types `Container<T>`, `Container<Container<T>>`, `...`
    can be used to represent N-dimensional arrays of `T`
    which *cannot* contain NULL values.

User-defined data conversions could be implemented by either:

  - overloading the operators `operator<<` and `operator>>` for
    [`std::ostream`][std_ostream] and [`std::istream`][std_istream]
    respectively;
  - specializing the template structure `dmitigr::pgfe::Conversions`.
    (With this approach overheads of standard IO streams can be avoided.)

Signal handling
---------------

Server signals are represented by classes, inherited from
`dmitigr::pgfe::Signal`:

  - signals that are server notices are represented by the class
    `dmitigr::pgfe::Notice`;
  - signals that are server notifications are represented by the class
    `dmitigr::pgfe::Notification`.

Signals can be handled:

  - synchronously, by using the signal handlers (see
    `dmitigr::pgfe::Connection::set_notice_handler()`,
    `dmitigr::pgfe::Connection::set_notification_handler()`);
  - asynchronously, by using the methods that provides access to the retrieved
    signals directly (see `dmitigr::pgfe::Connection::notice()`,
    `dmitigr::pgfe::Connection::notification()`).

Signal handlers, being set, called by
`dmitigr::pgfe::Connection::handle_signals()`. The latter is called automatically
while waiting a response. If no handler is set, corresponding signals will be
collected in the internal storage and *should* be popped up by using
`dmitigr::pgfe::Connection::pop_notice()` and/or
`dmitigr::pgfe::Connection::pop_notification()`.

**Be aware, that if signals are not popped up from the internal storage it may
cause memory exhaustion!**

Dynamic SQL
-----------

The standard classes like [`std::string`][std_string] or
[`std::ostringstream`][std_ostringstream] can be used to make SQL strings
dynamically. However, in some cases it is more convenient to use the class
`dmitigr::pgfe::Sql_string` for this purpose. Consider the following statement:

```sql
SELECT :expr::int, ':expr';
```

This SQL string has one named parameter `expr` and one string constant
`':expr'`. It's possible to replace the named parameters of the SQL string with
another SQL string by using `dmitigr::pgfe::Sql_string::replace_parameter()`,
for example:

```cpp
auto sql = dmitigr::pgfe::Sql_string::make("SELECT :expr::int, ':expr'");
sql->replace_parameter("expr", "sin(:expr1::int), cos(:expr2::int)");
```

Now the statement has two named parameters, and looks like:

```sql
SELECT sin(:expr1::int), cos(:expr2::int), ':expr'
```

Note, that the quoted string `:expr` is not affected by the replacement operation!

Working with SQL code separately of C++ code
--------------------------------------------

The idea of the approach is to store the SQL code in a separate place, such
as a text file. Consider the following SQL input, which is consists of two SQL
strings with an extra data specified by the [dollar-quoted][dollar-quoting]
string constants in the related comments:

```sql
-- This is query 1
--
-- $id$plus-one$id$
SELECT :n::int + 1, ';'; -- note, the semicolons in quotes are allowed!

/* This is query 2
 *
 * $id$minus-one$id$
 */
SELECT :n::int - 1
```
These SQL strings can be easily accessed by using the
`dmitigr::pgfe::Sql_vector` API, for example:

```cpp
std::string read_file(const std::filesystem::path& path);

void foo()
{
  namespace pgfe = dmitigr::pgfe;
  const auto input = read_file("bunch.sql");
  auto bunch = pgfe::Sql_vector::make(input);
  auto* minus_one = bunch->sql_string("id", "minus-one"); // SELECT :n::int - 1
  auto*  plus_one = bunch->sql_string("id",  "plus-one"); // SELECT :n::int + 1, ';'
  // ...
}
```

Exceptions
----------

Pgfe may throw:

  - an instance of the type [`std::logic_error`][std_logic_error] when:
      + API contract requirements are violated;
      + an assertion failure has occurred (it's possible only with the "debug"
      build of Pgfe);
  - an instance of the types [`std::runtime_error`][std_runtime_error] or
    `dmitigr::pgfe::Client_exception` when some kind of runtime error occured
    on the client side;
  - an instance of the type `dmitigr::pgfe::Server_exception` when some error
    occured on the server side and the methods like
    `dmitigr::pgfe::Connection::wait_response_throw()` is in use (which is the
    case when using `dmitigr::pgfe::Connection::perform()`,
    `dmitigr::pgfe::Connection::execute()` etc).

Thread safety
-------------

By default, if not explicitly documented, all functions and methods of Pgfe are
*not* thread safe. Thus, in most cases, some of the synchronization mechanisms
(like mutexes) must be used to work with the same object from several threads.

Dependencies
============

Pgfe is depends on the [libpq] library.

CMake options
=============

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

Copyright
=========

Copyright (C) [Dmitry Igrishin][dmitigr_mail]

[dmitigr_mail]: mailto:dmitigr@gmail.com
[dmitigr_cefeika]: https://github.com/dmitigr/cefeika.git
[dmitigr_cefeika_usage]: https://github.com/dmitigr/cefeika.git#usage
[dmitigr_pgfe]: https://github.com/dmitigr/pgfe.git
[dmitigr_pgfe_doc]: http://dmitigr.ru/en/projects/cefeika/pgfe/doc/
[dmitigr_pgfe_doc_diagram]: http://dmitigr.ru/en/projects/cefeika/pgfe/doc/dmitigr_pgfe_overview.violet.html

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
