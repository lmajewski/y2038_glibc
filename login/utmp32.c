/* Compability symbols for utmp with 32-bit entry times.
   Copyright (C) 2008-2020 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <utmp.h>
#include <errno.h>
#include <libc-lock.h>

#include <utmp32.h>
#include <utmp-convert.h>
#include <utmp-private.h>
#include <utmp-path.h>

#include <utmp-compat.h>
#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)

/* The compat utmp{x} functions are used for 32-bit time_t utmp/utmpx struct
   and they use two operation modes:

   1. Read/write 64-bit utmp{x} entries and convert them to/from 32-bit utmp.
      This is done for the exported UTMP_FILE / WUTMP_FILE
      (__libc_utmpname_mode equal to UTMPNAME_TIME64).

   2. Read/write 32-bit utmp if the target file is any other than the default
      UTMP_FILE / WUTMP_FILE ones.

   It allows maintaining the already set file format for old records, while
   also allowing reading newer ones (which might be created with call to the
   default utmp/utmpx symbol version).  */

int
__getutid32_r (const struct utmp32 *id, struct utmp32 *buffer,
	       struct utmp32 **result)
{
  int r;

  switch (id->ut_type)
    {
    case RUN_LVL:
    case BOOT_TIME:
    case OLD_TIME:
    case NEW_TIME:
    case INIT_PROCESS:
    case LOGIN_PROCESS:
    case USER_PROCESS:
    case DEAD_PROCESS:
      break;
    default:
      __set_errno (EINVAL);
      *result = NULL;
      return -1;
    }

  __libc_lock_lock (__libc_utmp_lock);

  if (__libc_utmpname_mode == UTMPNAME_TIME64)
   {
      struct utmp in64;
      struct utmp out64;
      struct utmp *out64p;

      __utmp_convert32to64 (id, &in64);

      r =  __libc_getutid_r (&in64, &out64, &out64p);
      if (r == 0)
	{
	  __utmp_convert64to32 (out64p, buffer);
	  *result = buffer;
	}
      else
	*result = NULL;
    }
  else
    r = __libc_getutid32_r (id, buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return r;
}
compat_symbol (libc, __getutid32_r, getutid_r, GLIBC_2_0);

struct utmp32 *
__getutid32 (const struct utmp32 *id)
{
  static struct utmp32 *out32 = NULL;
  if (out32 == NULL)
    {
      out32 = malloc (sizeof (struct utmp32));
      if (out32 == NULL)
	return NULL;
    }

  struct utmp32 *result;
  return __getutid32_r (id, out32, &result) < 0 ? NULL : result;
}
compat_symbol (libc, __getutid32, getutid, GLIBC_2_0);

int
__getutline32_r (const struct utmp32 *line,
		 struct utmp32 *buffer, struct utmp32 **result)
{
  int r;

  __libc_lock_lock (__libc_utmp_lock);

  if (__libc_utmpname_mode == UTMPNAME_TIME64)
   {
      struct utmp in64;
      struct utmp out64;
      struct utmp *out64p;

      __utmp_convert32to64 (line, &in64);

      r =  __libc_getutline_r (&in64, &out64, &out64p);
      if (r == 0)
	{
	  __utmp_convert64to32 (out64p, buffer);
	  *result = buffer;
	}
      else
	*result = NULL;
    }
  else
    r = __libc_getutline32_r (line, buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return r;
}
compat_symbol (libc, __getutline32_r, getutline_r, GLIBC_2_0);

struct utmp32 *
__getutline32 (const struct utmp32 *line)
{
  static struct utmp32 *out32 = NULL;
  if (out32 == NULL)
    {
      out32 = malloc (sizeof (struct utmp32));
      if (out32 == NULL)
	return NULL;
    }

  struct utmp32 *result;
  return __getutline32_r (line, out32, &result) < 0 ? NULL : result;
}
compat_symbol (libc, __getutline32, getutline, GLIBC_2_0);

struct utmp32 *
__pututline32 (const struct utmp32 *line)
{
  struct utmp32 *r;

  __libc_lock_lock (__libc_utmp_lock);

  if (__libc_utmpname_mode == UTMPNAME_TIME64)
   {
      struct utmp in64;
      __utmp_convert32to64 (line, &in64);
      struct utmp *out64p = __libc_pututline (&in64);
      r = out64p != NULL ? (struct utmp32 *) line : NULL;
    }
  else
    r = __libc_pututline32 (line);

  __libc_lock_unlock (__libc_utmp_lock);

  return r;
}
compat_symbol (libc, __pututline32, pututline, GLIBC_2_0);

int
__getutent32_r (struct utmp32 *buffer, struct utmp32 **result)
{
  int r;

  __libc_lock_lock (__libc_utmp_lock);

  if (__libc_utmpname_mode == UTMPNAME_TIME64)
    {
      struct utmp out64;
      struct utmp *out64p;
      r = __libc_getutent_r (&out64, &out64p);
      if (r == 0)
	{
	  __utmp_convert64to32 (out64p, buffer);
	  *result = buffer;
	}
    }
  else
    r = __libc_getutent32_r (buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return r;
}
compat_symbol (libc, __getutent32_r, getutent_r, GLIBC_2_0);

struct utmp32 *
__getutent32 (void)
{
  static struct utmp32 *out32 = NULL;
  if (out32 == NULL)
    {
      out32 = malloc (sizeof (struct utmp32));
      if (out32 == NULL)
	return NULL;
    }

  struct utmp32 *result;
  return __getutent32_r (out32, &result) < 0 ? NULL : result;
}
compat_symbol (libc, __getutent32, getutent, GLIBC_2_0);

void
__updwtmp32 (const char *wtmp_file, const struct utmp32 *utmp)
{
  const char *file_name = utmp_file_name_time32 (wtmp_file);

  if (__libc_utmpname_mode == UTMPNAME_TIME64)
    {
      struct utmp in32;
      __utmp_convert32to64 (utmp, &in32);
      __libc_updwtmp (file_name, &in32);
    }
  else
    __libc_updwtmp32 (file_name, utmp);
}
compat_symbol (libc, __updwtmp32, updwtmp, GLIBC_2_0);

#endif /* SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)   */
