// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/tests/unit.hpp"
#include "dmitigr/pgfe/error.hxx"
#include "dmitigr/pgfe/notice.hxx"

#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;
  try {
    const std::string severity_localized{"severity_localized"};

    const std::optional<std::string> notice_severity_non_localized{"WARNING"};
    const std::optional<std::string> error_severity_non_localized{"ERROR"};
    const std::string notice_sqlstate{"01000"};
    const std::string error_sqlstate{"0A000"};

    const std::string brief{"brief"};
    const std::optional<std::string> detail{"detail"};
    const std::optional<std::string> hint{"hint"};
    const std::optional<std::string> query_position{"query_position"};
    const std::optional<std::string> internal_query_position{"internal_query_position"};
    const std::optional<std::string> internal_query{"internal_query"};
    const std::optional<std::string> context{"context"};
    const std::optional<std::string> schema_name{"schema_name"};
    const std::optional<std::string> table_name{"table_name"};
    const std::optional<std::string> column_name{"column_name"};
    const std::optional<std::string> data_type_name{"data_type_name"};
    const std::optional<std::string> constraint_name{"constraint_name"};
    const std::optional<std::string> source_file{"source_file"};
    const std::optional<std::string> source_line{"source_line"};
    const std::optional<std::string> source_function{"source_function"};

    const auto test_problem = [&](const pgfe::Problem* const problem)
    {
      assert(problem);
      const auto severity = problem->severity();
      assert(severity == pgfe::Problem_severity::warning || severity == pgfe::Problem_severity::error);
      assert(severity != pgfe::Problem_severity::warning || problem->code() == pgfe::Server_errc::c01_warning);
      assert(severity != pgfe::Problem_severity::error   || problem->code() == pgfe::Server_errc::c0a_feature_not_supported);
      assert(problem->severity_localized() == severity_localized);
      assert(severity != pgfe::Problem_severity::warning || problem->severity_non_localized() == notice_severity_non_localized);
      assert(severity != pgfe::Problem_severity::error   || problem->severity_non_localized() == error_severity_non_localized);
      assert(severity != pgfe::Problem_severity::warning || problem->sqlstate() == notice_sqlstate);
      assert(severity != pgfe::Problem_severity::error   || problem->sqlstate() == error_sqlstate);
      assert(problem->brief() == brief);
      assert(problem->detail() == detail);
      assert(problem->hint() == hint);
      assert(problem->query_position() == query_position);
      assert(problem->internal_query_position() == internal_query_position);
      assert(problem->internal_query() == internal_query);
      assert(problem->context() == context);
      assert(problem->schema_name() == schema_name);
      assert(problem->table_name() == table_name);
      assert(problem->column_name() == column_name);
      assert(problem->data_type_name() == data_type_name);
      assert(problem->constraint_name() == constraint_name);
      assert(problem->source_file() == source_file);
      assert(problem->source_line() == source_line);
      assert(problem->source_function() == source_function);
    };

    {
      pgfe::detail::simple_Notice snotice(severity_localized,
        notice_severity_non_localized,
        notice_sqlstate,
        brief,
        detail,
        hint,
        query_position,
        internal_query_position,
        internal_query,
        context,
        schema_name,
        table_name,
        column_name,
        data_type_name,
        constraint_name,
        source_file,
        source_line,
        source_function);
      test_problem(&snotice);

      auto iproblem_copy = snotice.to_problem();
      assert(iproblem_copy);
      test_problem(iproblem_copy.get());

      auto inotice_copy = snotice.to_notice();
      assert(inotice_copy);
      test_problem(inotice_copy.get());
    }

    {
      pgfe::detail::simple_Error serror(severity_localized,
        error_severity_non_localized,
        error_sqlstate,
        brief,
        detail,
        hint,
        query_position,
        internal_query_position,
        internal_query,
        context,
        schema_name,
        table_name,
        column_name,
        data_type_name,
        constraint_name,
        source_file,
        source_line,
        source_function);
      test_problem(&serror);

      auto iproblem_copy = serror.to_problem();
      assert(iproblem_copy);
      test_problem(iproblem_copy.get());

      auto ierror_copy = serror.to_error();
      assert(ierror_copy);
      test_problem(ierror_copy.get());
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
