# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

set(dmitigr_cefeika_libraries
  misc
  os
  net
  pgfe
  )

set(dmitigr_cefeika_misc_deps)
set(dmitigr_cefeika_os_deps misc)
set(dmitigr_cefeika_net_deps os)
set(dmitigr_cefeika_pgfe_deps net)
