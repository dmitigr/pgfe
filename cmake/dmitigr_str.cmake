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
# Info
# ------------------------------------------------------------------------------

dmitigr_libs_set_library_info(str 0 0 0 "Standard string extensions")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_str_headers
  basics.hpp
  c_str.h
  c_str.hpp
  exceptions.hpp
  line.hpp
  numeric.hpp
  predicate.hpp
  sequence.hpp
  stream.hpp
  substr.hpp
  time.hpp
  transform.hpp
  )

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_LIBS_TESTS)
  set(dmitigr_str_tests test time)
  set(dmitigr_str_tests_target_link_libraries dmitigr_base)
endif()
