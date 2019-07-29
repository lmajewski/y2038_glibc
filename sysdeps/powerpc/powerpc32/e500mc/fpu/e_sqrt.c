/* Double-precision floating point square root.
   Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <math_private.h>
#include <fenv_libc.h>
#include <inttypes.h>

#include <sysdep.h>
#include <ldsodefs.h>

static const ieee_float_shape_type a_nan = {.word = 0x7fc00000 };
static const ieee_float_shape_type a_inf = {.word = 0x7f800000 };
static const float two108 = 3.245185536584267269e+32;
static const float twom54 = 5.551115123125782702e-17;
static const float half = 0.5;

/* The method is based on the descriptions in:

   _The Handbook of Floating-Pointer Arithmetic_ by Muller et al., chapter 5;
   _IA-64 and Elementary Functions: Speed and Precision_ by Markstein, chapter 9

   We find the actual square root and half of its reciprocal
   simultaneously.  */

#ifdef __STDC__
double
__ieee754_sqrt (double b)
#else
double
__ieee754_sqrt (b)
     double b;
#endif
{
  if (__builtin_expect (b > 0, 1))
    {
      double y, g, h, d, r;
      ieee_double_shape_type u;

      if (__builtin_expect (b != a_inf.value, 1))
        {
          fenv_t fe;

          fe = fegetenv_register ();

          u.value = b;

          relax_fenv_state ();

          __asm__ ("frsqrte %[estimate], %[x]\n"
                   : [estimate] "=f" (y) : [x] "f" (b));

          /* Following Muller et al, page 168, equation 5.20.

             h goes to 1/(2*sqrt(b))
             g goes to sqrt(b).

             We need three iterations to get within 1ulp.  */

          /* Indicate that these can be performed prior to the branch.  GCC
             insists on sinking them below the branch, however; it seems like
             they'd be better before the branch so that we can cover any latency
             from storing the argument and loading its high word.  Oh well.  */

          g = b * y;
          h = 0.5 * y;

          /* Handle small numbers by scaling.  */
          if (__builtin_expect ((u.parts.msw & 0x7ff00000) <= 0x02000000, 0))
            return __ieee754_sqrt (b * two108) * twom54;

#define FMADD(a_, c_, b_)                                               \
          ({ double __r;                                                \
          __asm__ ("fmadd %[r], %[a], %[c], %[b]\n"                     \
                   : [r] "=f" (__r) : [a] "f" (a_), [c] "f" (c_), [b] "f" (b_)); \
          __r;})
#define FNMSUB(a_, c_, b_)                                          \
          ({ double __r;                                                \
          __asm__ ("fnmsub %[r], %[a], %[c], %[b]\n"                     \
                   : [r] "=f" (__r) : [a] "f" (a_), [c] "f" (c_), [b] "f" (b_)); \
          __r;})

          r = FNMSUB (g, h, half);
          g = FMADD (g, r, g);
          h = FMADD (h, r, h);

          r = FNMSUB (g, h, half);
          g = FMADD (g, r, g);
          h = FMADD (h, r, h);

          r = FNMSUB (g, h, half);
          g = FMADD (g, r, g);
          h = FMADD (h, r, h);

          /* g is now +/- 1ulp, or exactly equal to, the square root of b.  */

          /* Final refinement.  */
          d = FNMSUB (g, g, b);

          fesetenv_register (fe);
          return FMADD (d, h, g);
        }
    }
  else if (b < 0)
    {
      /* For some reason, some PowerPC32 processors don't implement
         FE_INVALID_SQRT.  */
#ifdef FE_INVALID_SQRT
      feraiseexcept (FE_INVALID_SQRT);

      fenv_union_t u = { .fenv = fegetenv_register () };
      if ((u.l & FE_INVALID) == 0)
#endif
	feraiseexcept (FE_INVALID);
      b = a_nan.value;
    }
  return f_wash (b);
}
strong_alias (__ieee754_sqrt, __sqrt_finite)
