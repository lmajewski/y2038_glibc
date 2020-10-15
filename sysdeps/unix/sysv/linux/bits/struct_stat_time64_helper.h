/* Definition for helper to define struct stat with 64 bit time.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _BITS_STRUCT_STAT_TIME64_HELPER_H
#define _BITS_STRUCT_STAT_TIME64_HELPER_H	1

#include <bits/endian.h>

/* The definition should be equal to the 'struct __timespec64' internal
   layout.  */
#if BYTE_ORDER == BIG_ENDIAN
# define __fieldts64(name)					\
     __time64_t name; __int32_t :32; __int32_t name ## nsec
#else
# define __fieldts64(name)					\
     __time64_t name; __int32_t name ## nsec; __int32_t :32
#endif

# ifdef __USE_XOPEN2K8
# define st_atime st_atim.tv_sec
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
#  define __STAT64_TIME64_CONTENT  \
    struct timespec st_atim; \
    struct timespec st_mtim; \
    struct timespec st_ctim;
# else
#  define __STAT64_TIME64_CONTENT  \
	__fieldts64 (st_atime); \
	__fieldts64 (st_mtime); \
	__fieldts64 (st_ctime);
# endif /* __USE_XOPEN2K8  */

/* Content of internal __stat64_t64 struct.  */
#define __STAT64_T64_CONTENT                                                   \
    __dev_t st_dev;			/* Device.  */                         \
    __ino64_t st_ino;			/* file serial number.	*/             \
    __mode_t st_mode;			/* File mode.  */                      \
    __nlink_t st_nlink;			/* Link count.  */             \
    __uid_t st_uid;			/* User ID of the file's owner.  */    \
    __gid_t st_gid;			/* Group ID of the file's group.*/     \
    __dev_t st_rdev;			/* Device number, if device.  */       \
    __off64_t st_size;			/* Size of file, in bytes.  */         \
    __blksize_t st_blksize;		/* Optimal block size for I/O.  */     \
    __blkcnt64_t st_blocks;		/* Number 512-byte blocks allocated. */\
    __STAT64_TIME64_CONTENT

# undef __fieldts64
#endif /* _BITS_STRUCT_STAT_TIME64_HELPER_H  */
