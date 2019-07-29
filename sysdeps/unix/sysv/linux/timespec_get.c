/* Copyright (C) 2011-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <time.h>
#include <sysdep.h>
#include <errno.h>

#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

/* Set TS to calendar time based in time base BASE.  */
int
timespec_get (struct timespec *ts, int base)
{
  switch (base)
    {
      int res;
      INTERNAL_SYSCALL_DECL (err);
    case TIME_UTC:
      res = INTERNAL_VSYSCALL (clock_gettime, err, 2, CLOCK_REALTIME, ts);
      if (INTERNAL_SYSCALL_ERROR_P (res, err))
	return 0;
      break;

    default:
      return 0;
    }

  return base;
}

/* 64-bit time version */
int
__timespec_get64 (struct __timespec64 *ts64, int base)
{
  switch (base)
    {
    int result;
    case TIME_UTC:
	result = INLINE_SYSCALL (clock_gettime64, 2, CLOCK_REALTIME, ts64);
	if (result == 0)
		return base;

	if (result == -1 && errno == ENOSYS) {
		struct timespec ts32;
		result = timespec_get (&ts32, base);
		if (result == 0) {
			timespec_to_timespec64(&ts32, ts64);
			return base;
		}
	}
	break;

    default:
      return 0;
    }

  return 0;
}
