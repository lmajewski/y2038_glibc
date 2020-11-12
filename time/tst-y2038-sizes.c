/* Test for most important time related variables (structs)
   Copyright (C) 2020 Free Software Foundation, Inc.
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

/* Defines necessary for Y2038 time support on ports with
   __WORDSIZE == 32 and __TIMESIZE == 64.  */
#define _TIME_BITS 64
#define _FILE_OFFSET_BITS 64

#include <time.h>
#include <support/check.h>

static int
do_test (void)
{
  struct timespec t0;
  time_t t;

#if __TIMESIZE == 64 || defined __USE_TIME_BITS64
  TEST_COMPARE (sizeof(t), 8);
  TEST_COMPARE (sizeof(t0), 16);
  TEST_COMPARE (sizeof(t0.tv_sec), 8);
  TEST_COMPARE (sizeof(t0.tv_nsec), 8);
#else
  TEST_COMPARE (sizeof(t), 4);
  TEST_COMPARE (sizeof(t0), 8);
  TEST_COMPARE (sizeof(t0.tv_sec), 4);
  TEST_COMPARE (sizeof(t0.tv_nsec), 4);
#endif
  return 0;
}

#include <support/test-driver.c>
