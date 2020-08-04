/* Read a lastlog entry.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <utmp.h>
#include <stdbool.h>
#include <not-cancel.h>
#include <lastlog-compat.h>
#include <shlib-compat.h>

ssize_t
lastlog_read (const char *file, uid_t uid, struct lastlog *ll)
{
  int fd = __open_nocancel (file, O_RDONLY | O_LARGEFILE | O_CLOEXEC);
  if (fd == -1)
    return -1;

  size_t llsize;
  void *data;

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_33)
  struct lastlog_compat llcompat;
  if (is_path_lastlog_compat (file))
    {
      llsize = sizeof (struct lastlog_compat);
      data = &llcompat;
    }
  else
#endif
    {
      llsize = sizeof (struct lastlog);
      data = ll;
    }

  off64_t off = llsize * uid;
  ssize_t r = __pread64_nocancel (fd, data, llsize, off);
  __close_nocancel_nostatus (fd);

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_33)
  if (r == llsize && data == &llcompat)
    {
      ll->ll_time = llcompat.ll_time;
      memcpy (ll->ll_line, llcompat.ll_line, UT_LINESIZE);
      memcpy (ll->ll_host, llcompat.ll_host, UT_HOSTSIZE);
    }
#endif

  if (r == llsize)
    {
#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_33)
      if (data == &llcompat)
	{
	  ll->ll_time = llcompat.ll_time;
	  memcpy (ll->ll_line, llcompat.ll_line, UT_LINESIZE);
	  memcpy (ll->ll_host, llcompat.ll_host, UT_HOSTSIZE);
	}
#endif
      /* We need to return the expected 'struct lastlog' size in case of
	 success.  */
      r = sizeof (struct lastlog);
    }

  return r;
}
