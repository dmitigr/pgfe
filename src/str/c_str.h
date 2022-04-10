// -*- C -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_STR_C_STR_H
#define DMITIGR_STR_C_STR_H

#include <string.h>

#ifdef _WIN32

inline void *memmem(const void *const haystack, const size_t haystacklen,
    const void *const needle, const size_t needlelen)
{
  if (!haystack || !haystacklen || !needle || !needlelen)
    return NULL;

  const char* hay_ptr = (char *)haystack;
  const char* const hay_end = hay_ptr + haystacklen;
  const char* const search_end = hay_end - needlelen;
  for (; hay_ptr <= search_end; ++hay_ptr) {
    if (!memcmp(hay_ptr, needle, needlelen))
      return (void *)hay_ptr;
  }
  return NULL;
}

#endif

#endif  /* DMITIGR_STR_C_STR_H */
