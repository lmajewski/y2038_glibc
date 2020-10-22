#ifndef _SYS_MSG_H
#ifndef _ISOMAC
# include <time.h>
# include <sys/ipc.h>

/* Types used in the MSQID_DS structure definition.  */
typedef __syscall_ulong_t msgqnum_t;
typedef __syscall_ulong_t msglen_t;

struct msqid_ds
{
  struct ipc_perm msg_perm;	/* structure describing operation permission */
# if __TIMESIZE == 32
  __time_t msg_stime;		/* time of last msgsnd command */
  unsigned long int __msg_stime_high;
  __time_t msg_rtime;		/* time of last msgsnd command */
  unsigned long int __msg_rtime_high;
  __time_t msg_ctime;		/* time of last change */
  unsigned long int __msg_ctime_high;
# else
  __time_t msg_stime;		/* time of last msgsnd command */
  __time_t msg_rtime;		/* time of last msgsnd command */
  __time_t msg_ctime;		/* time of last change */
# endif
  __syscall_ulong_t __msg_cbytes; /* current number of bytes on queue */
  msgqnum_t msg_qnum;		/* number of messages currently on queue */
  msglen_t msg_qbytes;		/* max number of bytes allowed on queue */
  __pid_t msg_lspid;		/* pid of last msgsnd() */
  __pid_t msg_lrpid;		/* pid of last msgrcv() */
  __syscall_ulong_t __glibc_reserved4;
  __syscall_ulong_t __glibc_reserved5;
};

# ifdef __USE_MISC
#  define msg_cbytes	__msg_cbytes

/* ipcs ctl commands */
#  define MSG_STAT 11
#  define MSG_INFO 12
#  define MSG_STAT_ANY 13

/* buffer for msgctl calls IPC_INFO, MSG_INFO */
struct msginfo
  {
    int msgpool;
    int msgmap;
    int msgmax;
    int msgmnb;
    int msgmni;
    int msgssz;
    int msgtql;
    unsigned short int msgseg;
  };
# endif /* __USE_MISC */

# ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
#  define __ssize_t_defined
# endif

extern ssize_t __libc_msgrcv (int msqid, void *msgp, size_t msgsz,
			      long int msgtyp, int msgflg);
extern int __libc_msgsnd (int msqid, const void *msgp, size_t msgsz,
			  int msgflg);

# include <bits/types/struct_msqid64_ds.h>

# if __TIMESIZE == 64
#  define __msgctl64 __msgctl
# else
extern int __msgctl64 (int msqid, int cmd, struct __msqid64_ds *buf);
libc_hidden_proto (__msgctl64);
# endif
#else /* _ISOMAC = 1 - e.g. testsuite */
/* Provide exported sys/msg.h header */
# include <sysvipc/sys/msg.h>
#endif /* _ISOMAC */

#endif
