/* Converto to/from 64-bit to 32-bit time_t utmp/utmpx struct.
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

#include <string.h>
#include <utmp-convert.h>

/* Convert the 64 bit struct utmp value in FROM to the 32 bit version
   returned in TO.  */
void
__utmp_convert64to32 (const struct utmp *from, struct utmp32 *to)
{
  to->ut_type = from->ut_type;
  to->ut_pid = from->ut_pid;
  memcpy (to->ut_line, from->ut_line, UT_LINESIZE);
  memcpy (to->ut_user, from->ut_user, UT_NAMESIZE);
  memcpy (to->ut_id, from->ut_id, 4);
  memcpy (to->ut_host, from->ut_host, UT_HOSTSIZE);
  to->ut_exit = from->ut_exit;
  to->ut_session = from->ut_session;
  to->ut_tv.tv_sec = from->ut_tv.tv_sec;
  to->ut_tv.tv_usec = from->ut_tv.tv_usec;
  memcpy (to->ut_addr_v6, from->ut_addr_v6, 4 * 4);
}

/* Convert the 32 bit struct utmp value in FROM to the 64 bit version
   returned in TO.  */
void
__utmp_convert32to64 (const struct utmp32 *from, struct utmp *to)
{
  to->ut_type = from->ut_type;
  to->ut_pid = from->ut_pid;
  memcpy (to->ut_line, from->ut_line, UT_LINESIZE);
  memcpy (to->ut_user, from->ut_user, UT_NAMESIZE);
  memcpy (to->ut_id, from->ut_id, 4);
  memcpy (to->ut_host, from->ut_host, UT_HOSTSIZE);
  to->ut_exit = from->ut_exit;
  to->ut_session = from->ut_session;
  to->ut_tv.tv_sec = from->ut_tv.tv_sec;
  to->ut_tv.tv_usec = from->ut_tv.tv_usec;
  memcpy (to->ut_addr_v6, from->ut_addr_v6, sizeof (to->ut_addr_v6));
}
libc_hidden_def (__utmp_convert32to64)
