/* s390x already has 64-bit time for struct utmp{x} and lastlog.  This define
   disable the compat symbols and support to 32-bit entries.  */
#define UTMP_COMPAT_BASE GLIBC_2_0
