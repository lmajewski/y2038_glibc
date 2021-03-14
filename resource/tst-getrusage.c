/* Test for getrusage.
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

#include <time.h>
#include <errno.h>
#include <sys/resource.h>
#include <support/check.h>
#include <support/timespec.h>

static int
do_test (void)
{
  struct timespec tv_then, tv_now, r;
  struct rusage ru, ru_then;
  int ret;

  /* Check for invalid 'who' parameter.  */
  ret = getrusage (-100, &ru);
  if (! (ret == -1 && errno == EINVAL))
    FAIL_EXIT1 ("getrusage failed - EINVAL expected\n");

  /* Check if ru_stime.tv_usec and ru_utime.tv_usec are correct.  */
  ret = getrusage (RUSAGE_SELF, &ru_then);
  if (ret == -1)
    FAIL_EXIT1 ("getrusage failed: %m\n");

  TEST_VERIFY (ru_then.ru_utime.tv_usec >= 0);
  TEST_VERIFY (ru_then.ru_utime.tv_usec < 1000000);

  TEST_VERIFY (ru_then.ru_stime.tv_usec >= 0);
  TEST_VERIFY (ru_then.ru_stime.tv_usec < 1000000);

  tv_then = xclock_now (CLOCK_REALTIME);
  /* Busy wait for 10 miliseconds.  */
  do
    {
      tv_now = xclock_now (CLOCK_REALTIME);
      r = timespec_sub (tv_now, tv_then);
    }
  while (r.tv_nsec <= 10000000);

  ret = getrusage (RUSAGE_SELF, &ru);
  if (ret == -1)
    FAIL_EXIT1 ("getrusage failed: %m\n");

  TEST_VERIFY (ru.ru_utime.tv_sec >= ru_then.ru_utime.tv_sec);
  TEST_VERIFY (ru.ru_stime.tv_sec >= ru_then.ru_stime.tv_sec);

  return 0;
}

#include <support/test-driver.c>
