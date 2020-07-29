/* Tests for UTMP compat mode.
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

#include <shlib-compat.h>
#include <support/test-driver.h>
#include <stdio.h>

/* The test check the compat utmp/utmpx routines for the two operation modes:

   1. Use the default UTMP_FILE which uses the default code path which
      read/write 64-bit entries and converts to 32-bit time_t utmp/utmpx
      entries.

   2. Set a non default path with utmpname, which uses the compat code
      path to read/write 32-bit time_t utmp/utmpx entries.  */

#include <stdlib.h>
#include <string.h>
#include <utmp.h>

#include <support/check.h>
#include <support/temp_file.h>
#include <array_length.h>

#include <utmp32.h>

compat_symbol_reference (libc, setutent,  setutent,  GLIBC_2_0);
compat_symbol_reference (libc, pututline, pututline, GLIBC_2_0);
compat_symbol_reference (libc, getutline, getutline, GLIBC_2_0);
compat_symbol_reference (libc, getutent,  getutent,  GLIBC_2_0);
compat_symbol_reference (libc, getutid,   getutid,   GLIBC_2_0);

static struct utmp32 entry[] =
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

static time_t entry_time = 20000;
static pid_t entry_pid = 234;

static void
compare_utmp32 (const struct utmp32 *left, const struct utmp32 *right)
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

  for (int n = 0; n < array_length (entry); n++)
    TEST_VERIFY (pututline ((const struct utmp *) &entry[n]) != NULL);

  endutent ();
}

static void
do_check (void)
{
  int n;

  setutent ();

  n = 0;
  while (1)
    {
      struct utmp32 *ut = (struct utmp32 *) getutent ();
      if (ut == NULL)
	break;
      TEST_VERIFY (n <= array_length (entry));
      compare_utmp32 (ut, &entry[n]);
      n++;
    }
  TEST_COMPARE (n, array_length (entry));

  endutent ();
}

static void
simulate_login (const char *line, const char *user)
{
  for (int n = 0; n <array_length (entry); n++)
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

	  TEST_VERIFY (pututline ((const struct utmp *) &entry[n]) != NULL);

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

  for (n = 0; n < array_length (entry); n++)
    {
      if (strcmp (line, entry[n].ut_line) == 0)
	{
	  entry[n].ut_type = DEAD_PROCESS;
	  strncpy (entry[n].ut_user, "", sizeof (entry[n].ut_user));
          entry[n].ut_tv.tv_sec = (entry_time += 1000);
	  setutent ();

	  TEST_VERIFY (pututline ((const struct utmp *) &entry[n]) != NULL);

	  endutent ();

	  return;
	}
    }

  FAIL_EXIT1 ("no entries available");
}

static void
check_login (const char *line)
{
  struct utmp32 *up;
  struct utmp32 ut;

  setutent ();

  strcpy (ut.ut_line, line);
  up = (struct utmp32 *) getutline ((const struct utmp *) &ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (int n = 0; n < array_length (entry); n++)
    {
      if (strcmp (line, entry[n].ut_line) == 0)
	{
	  compare_utmp32 (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for line `%s'", line);
}

static void
check_logout (const char *line)
{
  struct utmp32 ut;

  setutent ();

  strcpy (ut.ut_line, line);
  TEST_VERIFY (getutline ((const struct utmp *) &ut) == NULL);

  endutent ();
}

static void
check_id (const char *id)
{
  struct utmp32 *up;
  struct utmp32 ut;

  setutent ();

  ut.ut_type = USER_PROCESS;
  strcpy (ut.ut_id, id);
  up = (struct utmp32 *) getutid ((const struct utmp *) &ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (int n = 0; n < array_length (entry); n++)
    {
      if (strcmp (id, entry[n].ut_id) == 0)
	{
	  compare_utmp32 (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for ID `%s'", id);
}

static void
check_type (int type)
{
  struct utmp32 *up;
  struct utmp32 ut;
  int n;

  setutent ();

  ut.ut_type = type;
  up = (struct utmp32 *) getutid ((const struct utmp *) &ut);
  TEST_VERIFY (up != NULL);

  endutent ();

  for (n = 0; n < array_length (entry); n++)
    {
      if (type == entry[n].ut_type)
	{
	  compare_utmp32 (up, &entry[n]);
	  return;
	}
    }

  FAIL_EXIT1 ("bogus entry for type `%d'", type);
}

static char *name;

static void
do_prepare (int argc, char **argv)
{
  int fd = create_temp_file ("tst-utmp32.", &name);
  TEST_VERIFY_EXIT (fd != -1);
}
#define PREPARE do_prepare

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
  run_test ();

  utmpname (name);
  run_test ();

  return 0;
}

#include <support/test-driver.c>
