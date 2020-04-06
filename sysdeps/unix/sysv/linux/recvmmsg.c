/* Copyright (C) 2010-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@redhat.com>, 2010.

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

#include <errno.h>
#include <sys/socket.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <socketcall.h>
#include <kernel-features.h>

int
__recvmmsg_time64 (int fd, struct mmsghdr *vmessages, unsigned int vlen,
                   int flags, struct __timespec64 *tmo)
{
#ifdef __ASSUME_RECVMMSG_SYSCALL
# ifdef __ASSUME_TIME64_SYSCALLS
#  ifndef __NR_recvmmsg_time64
#   define __NR_recvmmsg_time64 __NR_recvmmsg
#  endif
  return SYSCALL_CANCEL (recvmmsg_time64, fd, vmessages, vlen, flags, tmo);
# else
  int ret = SYSCALL_CANCEL (recvmmsg_time64, fd, vmessages, vlen, flags, tmo);
  if (ret == 0 || errno != ENOSYS)
    return ret;

  struct timespec ts32;
  if (tmo != NULL)
    {
      if (! in_time_t_range (tmo->tv_sec))
        {
          __set_errno (EOVERFLOW);
          return -1;
        }

      ts32 = valid_timespec64_to_timespec (*tmo);
    }

  return SYSCALL_CANCEL (recvmmsg, fd, vmessages, vlen, flags,
                         tmo != NULL ? &ts32 : NULL);
# endif
#else
  return SOCKETCALL_CANCEL (recvmmsg, fd, vmessages, vlen, flags, tmo);
#endif
}

#if __TIMESIZE != 64
libc_hidden_def (__recvmmsg_time64)

int
__recvmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags,
            struct timespec *tmo)
{
  struct __timespec64 ts64;

  if (tmo != NULL)
     ts64 = valid_timespec_to_timespec64 (*tmo);

  return __recvmmsg_time64 (fd, vmessages, vlen, flags,
                            tmo != NULL ? &ts64 : NULL);
}
#endif
strong_alias (__recvmmsg, recvmmsg)
