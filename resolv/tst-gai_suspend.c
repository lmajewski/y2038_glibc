/* Test for gai_suspend timeout
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
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <support/check.h>
#include <support/xtime.h>
#include <support/timespec.h>

static int
do_test (void)
{
  struct timespec ts;
  xclock_gettime (CLOCK_REALTIME, &ts);
  struct timespec timeout = make_timespec (1, 0);
  ts = timespec_add (ts, timeout);

  /* Ignore gaicb content - just wait for timeout.  */
  struct addrinfo result;

  struct gaicb gai = { "foo.baz", "0", NULL, &result };
  const struct gaicb * const gai_s[] = { &gai };
  struct gaicb *gai_p = &gai;

  int ret = getaddrinfo_a (GAI_NOWAIT, &gai_p, 1, NULL);
  if (ret != 0)
    FAIL_EXIT1 ("getaddrinfo_a failed: %m\n");

  ret = gai_suspend (gai_s, 1, &timeout);
  TEST_COMPARE (ret, EAI_AGAIN);
  TEST_TIMESPEC_NOW_OR_AFTER (CLOCK_REALTIME, ts);

  return 0;
}

#include <support/test-driver.c>
