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
    conn->for_each([](const auto* const row)
      {
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
  }  catch (const std::exception& e) {
    std::cerr << "Oops: " << e.what() << "\n";
  }
}
