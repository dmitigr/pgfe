# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

function(dmitigr_cefeika_load_with_deps component)
  #
  # Loading the dependencies
  #
  string(FIND "${component}" "thirdparty_" pos)
  if (pos EQUAL 0)
    set(comp "${component}")
  else()
    string(REGEX REPLACE "^([a-z0-9]+).*" "\\1" comp "${component}")
  endif()
  foreach(dep ${dmitigr_cefeika_${comp}_deps})
    dmitigr_cefeika_load_with_deps(${dep})
  endforeach()

  #
  # Loading the component
  #
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
    message(FATAL_ERROR "No configuration files of the Dmitigr Cefeika ${component} component found")
  endif()
endfunction()

# ------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/dmitigr_cefeika.cmake)

if(NOT dmitigr_cefeika_FIND_COMPONENTS)
  set(dmitigr_cefeika_FIND_COMPONENTS ${dmitigr_cefeika_libraries})
endif()

foreach(component ${dmitigr_cefeika_FIND_COMPONENTS})
  dmitigr_cefeika_load_with_deps(${component})
endforeach()
