/* xstat64 using Linux stat64 system call.
   Copyright (C) 1991-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <fcntl.h>
#include <sysdep.h>
#include <sys/syscall.h>

/* Get information about the file NAME in BUF.  */

int
___xstat64 (int vers, const char *name, struct stat64 *buf)
{
  int result;
  result = INLINE_SYSCALL (stat64, 2, name, buf);
  return result;
}


#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
versioned_symbol (libc, ___xstat64, __xstat64, GLIBC_2_2);
strong_alias (___xstat64, __old__xstat64)
compat_symbol (libc, __old__xstat64, __xstat64, GLIBC_2_1);
hidden_ver (___xstat64, __xstat64)
#else
strong_alias (___xstat64, __xstat64)
hidden_def (__xstat64)
#endif

/* 64-bit time version */

int
__xstat64_time64 (int vers, const char *name, struct __stat64_t64 *buf)
{
  int result;
  struct stat64 st64;

  result = INLINE_SYSCALL (stat64, 2, name, &st64);
  if (!result)
    {
      buf->st_dev          = st64.st_dev;
 #if defined _HAVE_STAT64___ST_INO
      buf->__st_ino        = st64.__st_ino;
 #endif
      buf->st_mode         = st64.st_mode;
      buf->st_nlink        = st64.st_nlink;
      buf->st_uid          = st64.st_uid;
      buf->st_gid          = st64.st_gid;
      buf->st_rdev         = st64.st_rdev;
      buf->st_size         = st64.st_size;
      buf->st_blksize      = st64.st_blksize;

      buf->st_blocks       = st64.st_blocks;

      buf->st_atim.tv_sec  = st64.st_atim.tv_sec;
      buf->st_atim.tv_nsec = st64.st_atim.tv_nsec;
      buf->st_atim.tv_pad = 0;
      buf->st_mtim.tv_sec  = st64.st_mtim.tv_sec;
      buf->st_mtim.tv_nsec = st64.st_mtim.tv_nsec;
      buf->st_mtim.tv_pad = 0;
      buf->st_ctim.tv_sec  = st64.st_ctim.tv_sec;
      buf->st_ctim.tv_nsec = st64.st_ctim.tv_nsec;
      buf->st_ctim.tv_pad = 0;

      buf->st_ino          = st64.st_ino;

      /* For Y2038 being safe we provide explicitly 64 bit time data types
	 via statx - as stat{64} syscall can only handle 32bits. */
#ifdef __NR_statx
      struct statx stx;
      result = INLINE_SYSCALL_CALL (statx, AT_FDCWD, name, 0,
				    STATX_ATIME | STATX_MTIME | STATX_CTIME,
				    &stx);
      if (!result) {
	      buf->st_atim.tv_sec  = stx.stx_atime.tv_sec;
	      buf->st_atim.tv_nsec = stx.stx_atime.tv_nsec;
	      buf->st_mtim.tv_sec  = stx.stx_mtime.tv_sec;
	      buf->st_mtim.tv_nsec = stx.stx_mtime.tv_nsec;
	      buf->st_ctim.tv_sec  = stx.stx_ctime.tv_sec;
	      buf->st_ctim.tv_nsec = stx.stx_ctime.tv_nsec;
      }
#endif
    }
  return result;
}
