/* Compability symbols for utmpx with 32-bit entry times.
   Copyright (C) 2008-2021 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <sys/types.h>
#include <string.h>
#include <utmp.h>
#include <errno.h>
#include <libc-symbols.h>

#include <utmp32.h>
#include <utmpx32.h>

#include <utmp-compat.h>
#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_1, UTMP_COMPAT_BASE)

# define CHECK_SIZE_AND_OFFSET(field) \
  _Static_assert (sizeof ((struct utmp32){0}.field)		\
		  == sizeof ((struct utmpx32){0}.field),		\
		  "sizeof ((struct utmp32){0}." #field " != "	\
		  "sizeof ((struct utmpx32){0}" #field);	\
  _Static_assert (offsetof (struct utmp32, field)			\
		  == offsetof (struct utmpx32, field),		\
		  "offsetof (struct utmp32, " #field ") != "	\
		  "offsetof (struct utmpx32, " #field ")");

/* Sanity check to call the utmp symbols.  */
_Static_assert (sizeof (struct utmpx32) == sizeof (struct utmp32),
		"sizeof (struct utmpx32) != sizeof (struct utmp32)");
CHECK_SIZE_AND_OFFSET (ut_type)
CHECK_SIZE_AND_OFFSET (ut_pid)
CHECK_SIZE_AND_OFFSET (ut_line)
CHECK_SIZE_AND_OFFSET (ut_user)
CHECK_SIZE_AND_OFFSET (ut_id)
CHECK_SIZE_AND_OFFSET (ut_host)
CHECK_SIZE_AND_OFFSET (ut_tv)

struct utmpx32 *
getutxent32 (void)
{
  return (struct utmpx32 *) __getutent32 ();
}
compat_symbol (libc, getutxent32, getutxent, GLIBC_2_1);

struct utmpx32 *
getutxid32 (const struct utmpx32 *id)
{
  return (struct utmpx32 *) __getutid32 ((const struct utmp32 *) id);
}
compat_symbol (libc, getutxid32, getutxid, GLIBC_2_1);

struct utmpx32 *
getutxline32 (const struct utmpx32 *line)
{
  return (struct utmpx32 *) __getutline32 ((const struct utmp32 *) line);
}
compat_symbol (libc, getutxline32, getutxline, GLIBC_2_1);

struct utmpx32 *
pututxline32 (const struct utmpx32 *utmpx)
{
  return (struct utmpx32 *) __pututline32 ((const struct utmp32 *) utmpx);
}
compat_symbol (libc, pututxline32, pututxline, GLIBC_2_1);

void
updwtmpx32 (const char *wtmpx_file, const struct utmpx32 *utmpx)
{
  __updwtmp32 (wtmpx_file, (const struct utmp32 *) utmpx);
}
compat_symbol (libc, updwtmpx32, updwtmpx, GLIBC_2_1);

#endif /* SHLIB_COMPAT(libc, GLIBC_2_1_1, UTMP_COMPAT_BASE)   */

#if SHLIB_COMPAT(libc, GLIBC_2_1_1, UTMP_COMPAT_BASE)

void
__getutmp32 (const struct utmpx32 *utmpx, struct utmp32 *utmp)
{
  memset (utmp, 0, sizeof (struct utmpx32));
  utmp->ut_type = utmpx->ut_type;
  utmp->ut_pid = utmpx->ut_pid;
  memcpy (utmp->ut_line, utmpx->ut_line, sizeof (utmp->ut_line));
  memcpy (utmp->ut_user, utmpx->ut_user, sizeof (utmp->ut_user));
  memcpy (utmp->ut_id, utmpx->ut_id, sizeof (utmp->ut_id));
  memcpy (utmp->ut_host, utmpx->ut_host, sizeof (utmp->ut_host));
  utmp->ut_tv.tv_sec = utmpx->ut_tv.tv_sec;
  utmp->ut_tv.tv_usec = utmpx->ut_tv.tv_usec;
}
compat_symbol (libc, __getutmp32, getutmp, GLIBC_2_1_1);

strong_alias (__getutmp32, __getutmpx32)
compat_symbol (libc, __getutmpx32, getutmpx, GLIBC_2_1_1);

#endif /* SHLIB_COMPAT(libc, GLIBC_2_1, UTMP_COMPAT_BASE)   */
