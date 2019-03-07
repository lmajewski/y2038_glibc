/* Copyright (C) 2015-2018 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/time.h>

#undef __gettimeofday

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
# define HAVE_VSYSCALL
#include <sysdep-vdso.h>
#endif

/* Get the current time of day and timezone information,
   putting it into *tv and *tz.  If tz is null, *tz is not filled.
   Returns 0 on success, -1 on errors.  */

int
__gettimeofday64 (struct __timeval64 *tv, struct timezone *tz)
{
  struct __timespec64 ts64;
  int result;

  if (tv) {
	  ts64.tv_pad = 0;
	  //#ifdef __vdso_clock_gettime
	  //	  result = INLINE_VSYSCALL (__clock_gettime64, 2, &ts64);
	  //#else
	  //	  result = INLINE_SYSCALL (__clock_gettime64, 2, &ts64);
	  //#endif
	  result = __clock_gettime64(CLOCK_REALTIME, &ts64);
	  if (result)
		  return result;

	  tv->tv_sec = ts64.tv_sec;
	  tv->tv_usec = ts64.tv_nsec / 1000;
  }

  if (tz) {
	  //#ifdef __vdso_gettimeofday
	  //	  result = INLINE_VSYSCALL (__clock_gettime64, 2, &ts64);
	  //#else
	  //	  result = INLINE_SYSCALL (__clock_gettime64, 2, &ts64);
	  //#endif
	  result = __gettimeofday(NULL, tz);
  }

  return result;
}
