/* Copyright (C) 1996-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <libc-lock.h>
#include <errno.h>
#include <stdlib.h>
#include <utmp.h>
#include <utmp-compat.h>
#include <shlib-compat.h>
#include <utmp-private.h>
#include <utmp-convert.h>

/* We have to use the lock in getutent_r.c.  */
__libc_lock_define (extern, __libc_utmp_lock attribute_hidden)


int
__getutid_r (const struct utmp *id, struct utmp *buffer, struct utmp **result)
{
  int retval;

  /* Test whether ID has any of the legal types.  */
  if (id->ut_type != RUN_LVL && id->ut_type != BOOT_TIME
      && id->ut_type != OLD_TIME && id->ut_type != NEW_TIME
      && id->ut_type != INIT_PROCESS && id->ut_type != LOGIN_PROCESS
      && id->ut_type != USER_PROCESS && id->ut_type != DEAD_PROCESS)
    /* No, using '<' and '>' for the test is not possible.  */
    {
      __set_errno (EINVAL);
      *result = NULL;
      return -1;
    }

  __libc_lock_lock (__libc_utmp_lock);

#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)
  if (__libc_utmpname_mode == UTMPNAME_TIME32)
    {
      struct utmp32 in32;
      struct utmp32 out32;
      struct utmp32 *out32p;

      __utmp_convert64to32 (id, &in32);

      retval =  __libc_getutid32_r (&in32, &out32, &out32p);
      if (retval == 0)
	{
	  __utmp_convert32to64 (out32p, buffer);
	  *result = buffer;
	}
      else
	*result = NULL;
    }
  else
#endif
    retval = __libc_getutid_r (id, buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return retval;
}
libc_hidden_def (__getutid_r)
#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)
versioned_symbol (libc, __getutid_r, getutid_r, UTMP_COMPAT_BASE);
#else
weak_alias (__getutid_r, getutid_r)
#endif
