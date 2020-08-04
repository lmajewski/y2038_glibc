/* Handle {u,w}tmp and {u,w}tmpx file name usage.
   Copyright (C) 1998-2021 Free Software Foundation, Inc.
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

#ifndef _UTMP_PATH_H
#define _UTMP_PATH_H 1

#include <string.h>
#include <unistd.h>

/* The function returns the utmp database for 32-bit utmp{x} entries based
   on FILE_NAME.  If the argument ends with 'x' and the file does not
   exits the default old utmp{x} name is returned instead.  */
static inline const char *
utmp_file_name_time32 (const char *file_name)
{
  if (strcmp (file_name, _PATH_UTMP_BASE "x") == 0
      && __access (_PATH_UTMP_BASE "x", F_OK) != 0)
    return _PATH_UTMP_BASE;
  else if (strcmp (file_name, _PATH_WTMP_BASE "x") == 0
	   && __access (_PATH_WTMP_BASE "x", F_OK) != 0)
    return _PATH_UTMP_BASE;

  return file_name;
}

#endif /* _UTMP_PATH_H  */
