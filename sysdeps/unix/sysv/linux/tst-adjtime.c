/* Test for adjtime.
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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/timespec.h>

#define TIMEOUT (60*10)
#define ADJTIME_LOOP_SLEEP 10000

static int
do_test (void)
{
  struct timespec tv_then, tv_now, tv_then_m, tv_now_m;
  struct timeval delta, odelta;
  int ret;

  /* Check if altering target time is allowed.  */
  if (getenv (SETTIME_ENV_NAME) == NULL)
    FAIL_UNSUPPORTED ("adjtime is executed only when "\
                      SETTIME_ENV_NAME" is set\n");

  tv_then = xclock_now (CLOCK_REALTIME);
  tv_then_m = xclock_now (CLOCK_MONOTONIC);

  /* Setup time value to adjust - 1 sec. */
  delta.tv_sec = 1;
  delta.tv_usec = 0;

  ret = adjtime (&delta, NULL);
  if (ret == -1)
    FAIL_EXIT1 ("adjtime failed: %m\n");

  /* As adjtime adjust clock time gradually by some small percentage
     of delta in each second one needs to check if this process has
     been finished.  */
  do
    {
      ret = adjtime (NULL, &odelta);
      if (ret == -1)
        FAIL_EXIT1 ("adjtime failed: %m\n");

      ret = usleep (ADJTIME_LOOP_SLEEP);
      if (ret != 0)
        FAIL_EXIT1 ("usleep failed: %m\n");
    }
  while ( ! (odelta.tv_sec == 0 && odelta.tv_usec == 0));

  tv_now = xclock_now (CLOCK_REALTIME);
  tv_now_m = xclock_now (CLOCK_MONOTONIC);

  /* Check if adjtime adjusted the system time.  */
  struct timespec r = timespec_sub (tv_now, tv_then);
  struct timespec r_m = timespec_sub (tv_now_m, tv_then_m);

  struct timespec r_adjtime = timespec_sub (r, r_m);

  TEST_COMPARE (support_timespec_check_in_range
                ((struct timespec) { 1, 0 }, r_adjtime, 0.9, 1.1), 1);

  return 0;
}

#include <support/test-driver.c>
