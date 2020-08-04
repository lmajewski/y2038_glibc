/* Tests for lastlog read/write functions.
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

#include <stdlib.h>
#include <string.h>
#include <utmp.h>

#include <support/check.h>
#include <support/temp_file.h>

#include <stdio.h>

static struct
{
  uid_t uid;
  struct lastlog ll;
} entry[] =
{
  { 0,    { .ll_time = 1000, .ll_line = "tty1" } },
  { 1000, { .ll_time = 2000, .ll_line = "pts/0", .ll_host = "127.0.0.1" } },
  { 20,   { .ll_time = 3000, .ll_line = "pts/1", .ll_host = "192.168.0.1" } },
  { 30,   { .ll_time = 4000, .ll_line = "tty2" } },
};
const size_t entry_size = sizeof (entry) / sizeof (entry[0]);

static void
run_test (const char *filename)
{
  for (int n = 0; n < entry_size; n++)
    TEST_COMPARE (lastlog_write (filename, entry[n].uid, &entry[n].ll),
		  sizeof (struct lastlog));

  for (int n = 0; n < entry_size; n++)
    {
      struct lastlog ll;
      TEST_COMPARE (lastlog_read (filename, entry[n].uid, &ll),
		    sizeof (struct lastlog));
      TEST_COMPARE (ll.ll_time, entry[n].ll.ll_time);
      TEST_COMPARE_BLOB (ll.ll_line, UT_LINESIZE,
			 entry[n].ll.ll_line, UT_LINESIZE);
      TEST_COMPARE_BLOB (ll.ll_host, UT_HOSTSIZE,
			 entry[n].ll.ll_host, UT_HOSTSIZE);
    }

  /* Check with an non present uid.  */
  {
    struct lastlog ll;
    TEST_COMPARE (lastlog_read (filename, 4000, &ll), 0);
  }
}

static int
do_test (void)
{
  /* The path triggers the read/write of compat (32-bit ll_time) entries
     for ABI that used to support it.  */
  run_test ("/var/run/lastlog");

  /* Any other file handles new (64-bit ll_time) entries.  */
  run_test ("/var/run/lastlog.v2");

  return 0;
}

#include <support/test-driver.c>
