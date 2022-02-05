# -*- cmake -*-
# Copyright (C) 2022 Dmitry Igrishin
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Dmitry Igrishin
# dmitigr@gmail.com

# ------------------------------------------------------------------------------
# Library list
# ------------------------------------------------------------------------------

set(dmitigr_cpplipa_libraries_all
  # Level 0
  base
  # Level 1
  concur dt fs hsh math os que rajson rnd str ttpl util
  # Level 2
  mulf net progpar sqlixx url
  # Level 3
  jrpc srv
  # Level 4
  fcgi http pgfe ws
  # Level 5
  web
  )

# ------------------------------------------------------------------------------
# Dependency lists
# ------------------------------------------------------------------------------

# Third-parties
set(dmitigr_cpplipa_3rdparty_uwebsockets_deps 3rdparty_usockets)

# Level 1
set(dmitigr_cpplipa_concur_deps base)
set(dmitigr_cpplipa_dt_deps base)
set(dmitigr_cpplipa_fs_deps)
set(dmitigr_cpplipa_hsh_deps)
set(dmitigr_cpplipa_math_deps base)
set(dmitigr_cpplipa_os_deps base)
set(dmitigr_cpplipa_que_deps)
set(dmitigr_cpplipa_rajson_deps base 3rdparty_rapidjson)
set(dmitigr_cpplipa_rnd_deps base)
set(dmitigr_cpplipa_str_deps base)
set(dmitigr_cpplipa_ttpl_deps base)
set(dmitigr_cpplipa_util_deps)
# Level 2
set(dmitigr_cpplipa_mulf_deps base str)
set(dmitigr_cpplipa_net_deps base fs os util)
set(dmitigr_cpplipa_progpar_deps base fs)
set(dmitigr_cpplipa_sqlixx_deps base fs)
set(dmitigr_cpplipa_url_deps base str)
# Level 3
set(dmitigr_cpplipa_jrpc_deps base math rajson str)
set(dmitigr_cpplipa_srv_deps base fs os progpar)
# Level 4
set(dmitigr_cpplipa_fcgi_deps base fs math net)
set(dmitigr_cpplipa_http_deps base dt net str)
set(dmitigr_cpplipa_pgfe_deps base fs net str util)
set(dmitigr_cpplipa_ws_deps base fs net 3rdparty_uwebsockets)
# Level 5
set(dmitigr_cpplipa_web_deps base fcgi fs http jrpc mulf str ttpl)
