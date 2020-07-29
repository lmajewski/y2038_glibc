/* Copyright (C) 2008-2021 Free Software Foundation, Inc.
   Contributed by Andreas Krebbel <Andreas.Krebbel@de.ibm.com>.
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


/* This file provides functions converting between the 32 and 64 bit
   struct utmp variants.  */

#ifndef _UTMP_CONVERT_H
#define _UTMP_CONVERT_H 1

#include <utmp32.h>

/* Convert the 64 bit struct utmp value in FROM to the 32 bit version
   returned in TO.  */
void __utmp_convert64to32 (const struct utmp *from, struct utmp32 *to)
  attribute_hidden;

/* Convert the 32 bit struct utmp value in FROM to the 64 bit version
   returned in TO.  */
void __utmp_convert32to64 (const struct utmp32 *from, struct utmp *to);
libc_hidden_proto (__utmp_convert32to64);

#endif /* utmp-convert.h */
