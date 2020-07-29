/* Tests for UTMP functions using default and old UTMP_FILE.
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

/* The default utmp{x} functions read and write the exported 64-bit time_t
   utmp/utmpx struct as default with the only exception when the old
   UTMP_FILE ('/var/run/utmp') or WUTMP_FILE ('/var/run/wtmp') is set
   explicitly with utmpname or updwtmp.  In this case old 32-bit time_t
   entries are read / write instead.  */

static struct utmp entry[] =
{
#define UT(a)  .ut_tv = { .tv_sec = (a)}

  { .ut_type = BOOT_TIME, .ut_pid = 1, UT(1000) },
  { .ut_type = RUN_LVL, .ut_pid = 1, UT(2000) },
  { .ut_type = INIT_PROCESS, .ut_pid = 5, .ut_id = "si", UT(3000) },
  { .ut_type = LOGIN_PROCESS, .ut_pid = 23, .ut_line = "tty1", .ut_id = "1",
    .ut_user = "LOGIN", UT(4000) },
  { .ut_type = USER_PROCESS, .ut_pid = 24, .ut_line = "tty2", .ut_id = "2",
    .ut_user = "albert", UT(8000) },
  { .ut_type = USER_PROCESS, .ut_pid = 196, .ut_line = "ttyp0", .ut_id = "p0",
    .ut_user = "niels", UT(10000) },
  { .ut_type = DEAD_PROCESS, .ut_line = "ttyp1", .ut_id = "p1", UT(16000) },
  { .ut_type = EMPTY },
  { .ut_type = EMPTY }
};
const size_t entry_size = sizeof (entry) / sizeof (entry[0]);

static time_t entry_time = 20000;
static pid_t entry_pid = 234;

static void
compare_utmp (const struct utmp *left, const struct utmp *right)
{
  TEST_COMPARE (left->ut_type, right->ut_type);
  TEST_COMPARE (left->ut_pid, right->ut_pid);
  TEST_COMPARE_BLOB (left->ut_line, sizeof (left->ut_line),
		     right->ut_line, sizeof (right->ut_line));
  TEST_COMPARE_BLOB (left->ut_id, sizeof (left->ut_id),
		     right->ut_id, sizeof (right->ut_id));
  TEST_COMPARE_BLOB (left->ut_user, sizeof (left->ut_user),
		     right->ut_user, sizeof (right->ut_user));
  TEST_COMPARE_BLOB (left->ut_host, sizeof (left->ut_host),
		     right->ut_host, sizeof (right->ut_host));
  TEST_COMPARE (left->ut_exit.e_termination, right->ut_exit.e_termination);
  TEST_COMPARE (left->ut_exit.e_exit, right->ut_exit.e_exit);
  TEST_COMPARE (left->ut_tv.tv_sec, right->ut_tv.tv_sec);
  TEST_COMPARE (left->ut_tv.tv_usec, right->ut_tv.tv_usec);
  TEST_COMPARE_BLOB (left->ut_addr_v6, sizeof (left->ut_addr_v6),
		     right->ut_addr_v6, sizeof (right->ut_addr_v6));
}

static void
do_init (void)
{
  setutent ();

  for (int n = 0; n < entry_size; n++)
    TEST_VERIFY (pututline (&entry[n]) != NULL);

  endutent ();
}

static void
do_check (void)
{
  struct utmp *ut;
  int n;

  setutent ();

  n = 0;
  while ((ut = getutent ()))
    {
      TEST_VERIFY (n <= entry_size);
      compare_utmp (ut, &entry[n]);
      n++;
    }
  TEST_COMPARE (n, entry_size);

  endutent ();
}

static void
simulate_login (const char *line, const char *user)
{
  for (int n = 0; n < entry_size; n++)
    {
      if (strcmp (line, entry[n].ut_line) == 0
	  || entry[n].ut_type == DEAD_PROCESS)
	{
	  if (entry[n].ut_pid == DEAD_PROCESS)
	    entry[n].ut_pid = (entry_pid += 27);
	  entry[n].ut_type = USER_PROCESS;
	  strncpy (entry[n].ut_user, user, sizeof (entry[n].ut_user));
	  entry[n].ut_tv.tv_sec = (entry_time += 1000);
	  setutent ();

	  TEST_VERIFY (pututline (&entry[n]) != NULL);

	  endutent ();

	  return;
	}
    }

  FAIL_EXIT1 ("no entries available");
}

static void
simulate_logout (const char *line)
{
  int n;

  for (n = 0; n < entry_size; n++)
    {
      if (strcmp (line, entry[n].ut_line) == 0)
	{
	  entry[n].ut_type = DEAD_PROCESS;
	  strncpy (entry[n].ut_user, "", sizeof (entry[n].ut_user));
          entry[n].ut_tv.tv_sec = (entry_time += 1000);
	  setutent ();

	  TEST_VERIFY (pututline (&entry[n]) != NULL);

	  endutent ();

	  return;
	}
    }

  FAIL_EXIT1 ("no entries available");
}

static void
check_login (const char *line)
{
  struct utmp *up;
  struct utmp ut;

  setutent ();

  strcpy (ut.ut_line, line);
  up = getutline (&ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (int n = 0; n < entry_size; n++)
    {
      if (strcmp (line, entry[n].ut_line) == 0)
	{
	  compare_utmp (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for line `%s'", line);
}

static void
check_logout (const char *line)
{
  struct utmp ut;

  setutent ();

  strcpy (ut.ut_line, line);
  TEST_VERIFY (getutline (&ut) == NULL);

  endutent ();
}

static void
check_id (const char *id)
{
  struct utmp *up;
  struct utmp ut;

  setutent ();

  ut.ut_type = USER_PROCESS;
  strcpy (ut.ut_id, id);
  up = getutid (&ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (int n = 0; n < entry_size; n++)
    {
      if (strcmp (id, entry[n].ut_id) == 0)
	{
	  compare_utmp (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for ID `%s'", id);
}

static void
check_type (int type)
{
  struct utmp *up;
  struct utmp ut;
  int n;

  setutent ();

  ut.ut_type = type;
  up = getutid (&ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (n = 0; n < entry_size; n++)
    {
      if (type == entry[n].ut_type)
	{
	  compare_utmp (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for type `%d'", type);
}

static void
run_test (void)
{
  do_init ();
  do_check ();

  simulate_login ("tty1", "erwin");
  do_check ();

  simulate_login ("ttyp1", "paul");
  do_check ();

  simulate_logout ("tty2");
  do_check ();

  simulate_logout ("ttyp0");
  do_check ();

  simulate_login ("ttyp2", "richard");
  do_check ();

  check_login ("tty1");
  check_logout ("ttyp0");
  check_id ("p1");
  check_id ("2");
  check_id ("si");
  check_type (BOOT_TIME);
  check_type (RUN_LVL);
}

static int
do_test (void)
{
  utmpname (UTMP_FILE);
  run_test ();

  utmpname ("/var/run/utmp");
  run_test ();

  return 0;
}

#include <support/test-driver.c>
