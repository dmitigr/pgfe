// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/os/proc_run.hpp>

#include <iostream>

namespace proc = dmitigr::os::proc;

void start()
{
  std::clog << "The process is started!" << std::endl;
  std::clog << "Start flag is " << proc::is_running << std::endl;
}

int main(int argc, char* argv[]) try {
  using namespace dmitigr::testo;
  proc::prog_params = {argc, argv};
  const std::string info = "[--detach]";
  const auto [detach_o, all] = proc::prog_params.options("detach");
  if (!all || !proc::prog_params.arguments().empty())
    proc::usage(info);

  const bool detach = detach_o.is_valid_throw_if_value();
  proc::start(detach, start);
 } catch (const std::exception& e) {
  std::clog << argv[0] << ": " << e.what() << std::endl;
 }
