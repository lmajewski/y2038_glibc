/* Struct stat with 64-bit time support.
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

#ifndef _BITS_STRUCT_STAT_TIME64_H
#define _BITS_STRUCT_STAT_TIME64_H 1

#if __TIMESIZE == 64
# define __stat64_t64 stat64
#else
# ifdef __USE_LARGEFILE64
#  include <struct___timespec64.h>
#  include <bits/struct_stat_time64_helper.h>
#  ifdef __USE_XOPEN2K8
#   undef __STAT64_TIME64_CONTENT
/* Use glibc internal types for time related members */
#   define __STAT64_TIME64_CONTENT  \
      struct __timespec64 st_atim; \
      struct __timespec64 st_mtim; \
      struct __timespec64 st_ctim;
#  endif /* __USE_XOPEN2K8 */
struct __stat64_t64
  {
    __STAT64_T64_CONTENT
  };

#   define _STATBUF_ST_BLKSIZE
#   define _STATBUF_ST_RDEV
#   define _STATBUF_ST_NSEC

#  endif /* __USE_LARGEFILE64  */
# endif /* __TIMESIZE == 64  */
#endif /* _BITS_STRUCT_STAT_TIME64_H  */
