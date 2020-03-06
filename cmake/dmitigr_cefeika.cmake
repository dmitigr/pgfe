# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

set(dmitigr_cefeika_libraries
  # Abstraction level 1
  util
  # Abstraction level 2
  fs
  mem
  os
  str
  # Abstraction level 3
  net
  # Abstraction level 4
  pgfe
  )

set(dmitigr_cefeika_util_deps)
set(dmitigr_cefeika_fs_deps util)
set(dmitigr_cefeika_mem_deps util)
set(dmitigr_cefeika_os_deps fs util)
set(dmitigr_cefeika_str_deps util)
set(dmitigr_cefeika_net_deps fs util)
set(dmitigr_cefeika_pgfe_deps mem net str util)
