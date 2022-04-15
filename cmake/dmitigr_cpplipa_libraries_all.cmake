# -*- cmake -*-
#
# Copyright 2022 Dmitry Igrishin
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# ------------------------------------------------------------------------------
# Library list
# ------------------------------------------------------------------------------

set(dmitigr_cpplipa_libraries_all
  # Level 0 (base level)
  base
  # Level 1
  algo concur dt fs hsh math os que rajson rnd str ttpl util uv
  # Level 2
  mulf net prg sqlixx url
  # Level 3
  jrpc ws wscl
  # Level 4
  fcgi http pgfe
  # Level 5
  web
  )

# ------------------------------------------------------------------------------
# Dependency lists
# ------------------------------------------------------------------------------

# Third-parties
set(dmitigr_cpplipa_3rdparty_uwebsockets_deps 3rdparty_usockets)

# Level 1
set(dmitigr_cpplipa_algo_deps base)
set(dmitigr_cpplipa_concur_deps base)
set(dmitigr_cpplipa_dt_deps base)
set(dmitigr_cpplipa_fs_deps)
set(dmitigr_cpplipa_hsh_deps)
set(dmitigr_cpplipa_math_deps base)
set(dmitigr_cpplipa_os_deps base)
set(dmitigr_cpplipa_que_deps)
set(dmitigr_cpplipa_rajson_deps base fs 3rdparty_rapidjson)
set(dmitigr_cpplipa_rnd_deps base)
set(dmitigr_cpplipa_str_deps base)
set(dmitigr_cpplipa_ttpl_deps base)
set(dmitigr_cpplipa_util_deps base)
set(dmitigr_cpplipa_uv_deps base)
# Level 2
set(dmitigr_cpplipa_mulf_deps base str)
set(dmitigr_cpplipa_net_deps base fs os util)
set(dmitigr_cpplipa_prg_deps base fs os rajson str)
set(dmitigr_cpplipa_sqlixx_deps base fs)
set(dmitigr_cpplipa_url_deps base str)
# Level 3
set(dmitigr_cpplipa_jrpc_deps base algo math rajson str)
set(dmitigr_cpplipa_ws_deps base fs net 3rdparty_uwebsockets)
set(dmitigr_cpplipa_wscl_deps base net 3rdparty_uwsc)
# Level 4
set(dmitigr_cpplipa_fcgi_deps base fs math net)
set(dmitigr_cpplipa_http_deps base dt net str)
set(dmitigr_cpplipa_pgfe_deps base fs net str util)
# Level 5
set(dmitigr_cpplipa_web_deps base fcgi fs http jrpc mulf str ttpl)
