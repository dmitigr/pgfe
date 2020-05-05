# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

set(dmitigr_cefeika_libraries
  base

  mem
  os
  str
  testo

  net

  pgfe
  )

set(dmitigr_cefeika_base_deps)

set(dmitigr_cefeika_mem_deps base)
set(dmitigr_cefeika_os_deps base)
set(dmitigr_cefeika_str_deps base)
set(dmitigr_cefeika_testo_deps base)

set(dmitigr_cefeika_net_deps os base)

set(dmitigr_cefeika_pgfe_deps mem net str base)
