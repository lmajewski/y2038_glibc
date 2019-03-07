/* Set the current time of day and timezone information.

   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <sys/time.h>

int __settimeofday64(const struct __timeval64 *tv,
		       const struct timezone *tz)
{
  struct __timespec64 ts64;
  int ret;

  if (tv) {
	  ts64.tv_sec = tv->tv_sec;
	  ts64.tv_nsec = tv->tv_usec * 1000;
	  ts64.tv_pad = 0;

	  ret = __clock_settime64(CLOCK_REALTIME, &ts64);
	  if (ret)
		  return ret;
  }

  if (tz) {
	return __settimeofday(NULL, tz);
  }

  return 0;
}
