/* Compat lastlog definitions.  s390 definition.
   Copyright (C) 2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _LASTLOG_COMPAT_H
#define _LASTLOG_COMPAT_H 1

/* The s390 changes its lastlog to 64-bit on glibc 2.9 without any handling
   of old format.  The is_path_lastlog_compat assumes 64-bit as default.  */

struct lastlog_compat
{
  int64_t ll_time;
  char ll_line[UT_LINESIZE];
  char ll_host[UT_HOSTSIZE];
};

static inline bool
is_path_lastlog_compat (const char *file)
{
  return false;
}

#endif
