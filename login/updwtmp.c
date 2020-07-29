/* Copyright (C) 1997-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#include <utmp.h>
#include <string.h>
#include <unistd.h>
#include <utmp-private.h>
#include <utmp-path.h>
#include <utmp-compat.h>
#include <utmp-convert.h>
#include <shlib-compat.h>

void
__updwtmp (const char *wtmp_file, const struct utmp *utmp)
{
#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)
  if (strcmp (wtmp_file, _PATH_WTMP_BASE) == 0
      || strcmp (wtmp_file, _PATH_WTMP_BASE) == 0)
    {
      const char *file_name = utmp_file_name_time32 (wtmp_file);
      struct utmp32 in32;
      __utmp_convert64to32 (utmp, &in32);
      __libc_updwtmp32 (file_name, &in32);
    }
  else
#endif
    __libc_updwtmp (wtmp_file, utmp);
}
libc_hidden_def (__updwtmp)
#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)
versioned_symbol (libc, __updwtmp, updwtmp, UTMP_COMPAT_BASE);
#else
weak_alias (__updwtmp, updwtmp)
#endif
