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

function(dmitigr_cpplipa_load_with_deps component)
  # Loading the component's dependencies
  foreach(dep ${dmitigr_cpplipa_${component}_deps})
    dmitigr_cpplipa_load_with_deps(${dep})
  endforeach()

  # Loading the component
  string(REGEX MATCH "(shared|static|interface)$" suffix "${component}")
  if(NOT suffix)
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/dmitigr_${component}_shared-config.cmake)
      set(suffix "shared")
    elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/dmitigr_${component}_static-config.cmake)
      set(suffix "static")
    elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/dmitigr_${component}_interface-config.cmake)
      set(suffix "interface")
    endif()
    set(component "${component}_${suffix}")
  endif()
  set(config_file "${CMAKE_CURRENT_LIST_DIR}/dmitigr_${component}-config.cmake")
  if(EXISTS ${config_file})
    include(${config_file})
  else()
    message(FATAL_ERROR "No configuration files for dmitigr_${component} found")
  endif()
endfunction()

# ------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/dmitigr_cpplipa_libraries.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dmitigr_cpplipa_libraries_all.cmake)

if(NOT dmitigr_cpplipa_FIND_COMPONENTS)
  set(dmitigr_cpplipa_FIND_COMPONENTS ${dmitigr_cpplipa_libraries})
endif()

foreach(component ${dmitigr_cpplipa_FIND_COMPONENTS})
  dmitigr_cpplipa_load_with_deps(${component})
endforeach()
