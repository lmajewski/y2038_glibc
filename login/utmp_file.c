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

#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/param.h>
#include <not-cancel.h>

#include <utmp-private.h>
#include <utmp-path.h>
#include <utmp-compat.h>
#include <shlib-compat.h>


/* Descriptor for the file and position.  */
static int file_fd = -1;
static bool file_writable;
static off64_t file_offset;


/* The utmp{x} internal functions work on two operations modes

   1. Read/write 64-bit time utmp{x} entries using the exported
      'struct utmp{x}'

   2. Read/write 32-bit time utmp{x} entries using the old 'struct utmp32'

   The operation mode mainly change the register size and how to interpret
   the 'last_entry' buffered record.  */
enum operation_mode_t
{
  UTMP_TIME64,
  UTMP_TIME32
};
static enum operation_mode_t cur_mode = UTMP_TIME64;

enum
{
  utmp_buffer_size = MAX (sizeof (struct utmp), sizeof (struct utmp32))
};

/* Cache for the last read entry.  */
static char last_entry[utmp_buffer_size];

static inline size_t last_entry_size (enum operation_mode_t mode)
{
  return mode == UTMP_TIME64 ? sizeof (struct utmp) : sizeof (struct utmp32);
}

static inline short int last_entry_type (enum operation_mode_t mode)
{
  short int r;
  if (mode == UTMP_TIME32)
    memcpy (&r, last_entry + offsetof (struct utmp32, ut_type), sizeof (r));
  else
    memcpy (&r, last_entry + offsetof (struct utmp, ut_type), sizeof (r));
  return r;
}

static inline const char *last_entry_id (enum operation_mode_t mode)
{
  if (mode == UTMP_TIME64)
   return ((struct utmp *) (last_entry))->ut_id;
  return ((struct utmp32 *) (last_entry))->ut_id;
}

static inline const char *last_entry_line (enum operation_mode_t mode)
{
  if (mode == UTMP_TIME64)
   return ((struct utmp *) (last_entry))->ut_line;
  return ((struct utmp32 *) (last_entry))->ut_line;
}

/* Returns true if *ENTRY matches last_entry, based on data->ut_type.  */
static bool
matches_last_entry (enum operation_mode_t mode, short int type,
		    const char *id, const char *line)
{
  if (file_offset <= 0)
    /* Nothing has been read.  last_entry is stale and cannot match.  */
    return false;

  switch (type)
    {
    case RUN_LVL:
    case BOOT_TIME:
    case OLD_TIME:
    case NEW_TIME:
      /* For some entry types, only a type match is required.  */
      return type == last_entry_type (mode);
    default:
      /* For the process-related entries, a full match is needed.  */
      return (type == INIT_PROCESS
	      || type == LOGIN_PROCESS
	      || type == USER_PROCESS
	      || type == DEAD_PROCESS)
	&& (last_entry_type (mode) == INIT_PROCESS
	    || last_entry_type (mode) == LOGIN_PROCESS
	    || last_entry_type (mode) == USER_PROCESS
	    || last_entry_type (mode) == DEAD_PROCESS)
	&& (id[0] != '\0' && last_entry_id (mode)[0] != '\0'
	    ? strncmp (id, last_entry_id (mode), 4 * sizeof (char)) == 0
	    : (strncmp (line, last_entry_id (mode), UT_LINESIZE) == 0));
    }
}

/* Locking timeout.  */
#ifndef TIMEOUT
# define TIMEOUT 10
#endif

/* Do-nothing handler for locking timeout.  */
static void timeout_handler (int signum) {};


/* try_file_lock (LOCKING, FD, TYPE) returns true if the locking
   operation failed and recovery needs to be performed.

   file_unlock (FD) removes the lock (which must have been
   successfully acquired). */

static bool
try_file_lock (int fd, int type)
{
  /* Cancel any existing alarm.  */
  int old_timeout = alarm (0);

  /* Establish signal handler.  */
  struct sigaction old_action;
  struct sigaction action;
  action.sa_handler = timeout_handler;
  __sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  __sigaction (SIGALRM, &action, &old_action);

  alarm (TIMEOUT);

  /* Try to get the lock.  */
 struct flock64 fl =
   {
    .l_type = type,
    .l_whence = SEEK_SET,
   };

 bool status = __fcntl64_nocancel (fd, F_SETLKW, &fl) < 0;
 int saved_errno = errno;

 /* Reset the signal handler and alarm.  We must reset the alarm
    before resetting the handler so our alarm does not generate a
    spurious SIGALRM seen by the user.  However, we cannot just set
    the user's old alarm before restoring the handler, because then
    it's possible our handler could catch the user alarm's SIGARLM and
    then the user would never see the signal he expected.  */
  alarm (0);
  __sigaction (SIGALRM, &old_action, NULL);
  if (old_timeout != 0)
    alarm (old_timeout);

  __set_errno (saved_errno);
  return status;
}

static void
file_unlock (int fd)
{
  struct flock64 fl =
    {
      .l_type = F_UNLCK,
    };
  __fcntl64_nocancel (fd, F_SETLKW, &fl);
}

static bool
internal_setutent (enum operation_mode_t mode)
{
  if (file_fd < 0)
    {
      const char *file_name = mode == UTMP_TIME64
	?__libc_utmp_file_name
	: utmp_file_name_time32 (__libc_utmp_file_name);

      file_writable = false;
      file_fd = __open_nocancel
	(file_name, O_RDONLY | O_LARGEFILE | O_CLOEXEC);
      if (file_fd == -1)
	return false;
      cur_mode = mode;
    }

  __lseek64 (file_fd, 0, SEEK_SET);
  file_offset = 0;

  return true;
}

/* Preform initialization if necessary.  */
static bool
maybe_setutent (enum operation_mode_t mode)
{
  if (file_fd >= 0 && cur_mode != mode)
    {
      __close_nocancel_nostatus (file_fd);
      file_fd = -1;
      file_offset = 0;
    }
  return file_fd >= 0 || internal_setutent (mode);
}

/* Reads the entry at file_offset, storing it in last_entry and
   updating file_offset on success.  Returns -1 for a read error, 0
   for EOF, and 1 for a successful read.  last_entry and file_offset
   are only updated on a successful and complete read.  */
static ssize_t
read_last_entry (enum operation_mode_t mode)
{
  char buffer[utmp_buffer_size];
  const size_t size = last_entry_size (mode);
  ssize_t nbytes = __pread64_nocancel (file_fd, &buffer, size, file_offset);
  if (nbytes < 0)
    return -1;
  else if (nbytes != size)
    /* Assume EOF.  */
    return 0;
  else
    {
      memcpy (last_entry, buffer, size);
      file_offset += size;
      return 1;
    }
}

static int
internal_getutent_r (enum operation_mode_t mode, void *buffer)
{
  int saved_errno = errno;

  if (!maybe_setutent (mode)
      || try_file_lock (file_fd, F_RDLCK))
    return -1;

  ssize_t nbytes = read_last_entry (mode);
  file_unlock (file_fd);

  if (nbytes <= 0)		/* Read error or EOF.  */
    {
      if (nbytes == 0)
	/* errno should be unchanged to indicate success.  A premature
	   EOF is treated like an EOF (missing complete record at the
	   end).  */
	__set_errno (saved_errno);
      return -1;
    }

  memcpy (buffer, &last_entry, last_entry_size (mode));
  return 0;
}

/* Search for *ID, updating last_entry and file_offset.  Return 0 on
   success and -1 on failure.  Does not perform locking; for that see
   internal_getut_r below.  */
static bool
internal_getut_nolock (enum operation_mode_t mode, short int type,
		       const char *id, const char *line)
{
  while (true)
    {
      ssize_t nbytes = read_last_entry (mode);
      if (nbytes < 0)
	return false;
      if (nbytes == 0)
	{
	  /* End of file reached.  */
	  __set_errno (ESRCH);
	  return false;
	}

      if (matches_last_entry (mode, type, id, line))
	break;
    }
  return true;
}

/* Search for *ID, updating last_entry and file_offset.  Return 0 on
   success and -1 on failure.  If the locking operation failed, write
   true to *LOCK_FAILED.  */
static bool
internal_getut_r (enum operation_mode_t mode, short int type, const char *id,
		  const char *line)
{
  if (try_file_lock (file_fd, F_RDLCK))
    return false;

  bool r = internal_getut_nolock (mode, type, id, line);
  file_unlock (file_fd);
  return r;
}

static int
internal_getutid_r (enum operation_mode_t mode, short int type,
		    const char *id, const char *line, void *buffer)
{
  if (!maybe_setutent (mode))
    return -1;

  /* We don't have to distinguish whether we can lock the file or
     whether there is no entry.  */
  if (! internal_getut_r (mode, type, id, line))
    return -1;

  memcpy (buffer, &last_entry, last_entry_size (mode));

  return 0;
}

/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
static int
internal_getutline_r (enum operation_mode_t mode, const char *line,
		      void *buffer)
{
  if (!maybe_setutent (mode)
      || try_file_lock (file_fd, F_RDLCK))
    return -1;

  while (1)
    {
      ssize_t nbytes = read_last_entry (mode);
      if (nbytes < 0)
	{
	  file_unlock (file_fd);
	  return -1;
	}
      if (nbytes == 0)
	{
	  /* End of file reached.  */
	  file_unlock (file_fd);
	  __set_errno (ESRCH);
	  return -1;
	}

      /* Stop if we found a user or login entry.  */
      if ((last_entry_type (mode) == USER_PROCESS
	   || last_entry_type (mode) == LOGIN_PROCESS)
	  && (strncmp (line, last_entry_line (mode), UT_LINESIZE) == 0))
	break;
    }

  file_unlock (file_fd);
  memcpy (buffer, &last_entry, last_entry_size (mode));

  return 0;
}

static bool
internal_pututline (enum operation_mode_t mode, short int type,
		    const char *id, const char *line, const void *data)
{
  if (!maybe_setutent (mode))
    return false;

  if (! file_writable)
    {
      /* We must make the file descriptor writable before going on.  */
      const char *file_name = mode == UTMP_TIME64
	? __libc_utmp_file_name
	: utmp_file_name_time32 (__libc_utmp_file_name);

      int new_fd = __open_nocancel
	(file_name, O_RDWR | O_LARGEFILE | O_CLOEXEC);
      if (new_fd == -1)
	return false;

      if (__dup2 (new_fd, file_fd) < 0)
	{
	  __close_nocancel_nostatus (new_fd);
	  return false;
	}
      __close_nocancel_nostatus (new_fd);
      file_writable = true;
    }

  /* Exclude other writers before validating the cache.  */
  if (try_file_lock (file_fd, F_WRLCK))
    return false;

  /* Find the correct place to insert the data.  */
  const size_t utmp_size = last_entry_size (mode);
  bool found = false;
  if (matches_last_entry (mode, type, id, line))
    {
      /* Read back the entry under the write lock.  */
      file_offset -= utmp_size;
      ssize_t nbytes = read_last_entry (mode);
      if (nbytes < 0)
	{
	  file_unlock (file_fd);
	  return false;
	}

      if (nbytes == 0)
	/* End of file reached.  */
	found = false;
      else
	found = matches_last_entry (mode, type, id, line);
    }

  if (!found)
    /* Search forward for the entry.  */
    found = internal_getut_nolock (mode, type, id, line);

  off64_t write_offset;
  if (!found)
    {
      /* We append the next entry.  */
      write_offset = __lseek64 (file_fd, 0, SEEK_END);

      /* Round down to the next multiple of the entry size.  This
	 ensures any partially-written record is overwritten by the
	 new record.  */
      write_offset = write_offset / utmp_size * utmp_size;
    }
  else
    /* Overwrite last_entry.  */
    write_offset = file_offset - utmp_size;

  /* Write the new data.  */
  ssize_t nbytes;
  if (__lseek64 (file_fd, write_offset, SEEK_SET) < 0
      || (nbytes = __write_nocancel (file_fd, data, utmp_size)) < 0)
    {
      /* There is no need to recover the file position because all
	 reads use pread64, and any future write is preceded by
	 another seek.  */
      file_unlock (file_fd);
      return false;
    }

  if (nbytes != utmp_size)
    {
      /* If we appended a new record this is only partially written.
	 Remove it.  */
      if (!found)
	(void) __ftruncate64 (file_fd, write_offset);
      file_unlock (file_fd);
      /* Assume that the write failure was due to missing disk
	 space.  */
      __set_errno (ENOSPC);
      return false;
    }

  file_unlock (file_fd);
  file_offset = write_offset + utmp_size;

  return true;
}

static int
internal_updwtmp (enum operation_mode_t mode, const char *file,
		  const void *utmp)
{
  int result = -1;
  off64_t offset;
  int fd;

  /* Open WTMP file.  */
  fd = __open_nocancel (file, O_WRONLY | O_LARGEFILE);
  if (fd < 0)
    return -1;

  if (try_file_lock (fd, F_WRLCK))
    {
      __close_nocancel_nostatus (fd);
      return -1;
    }

  /* Remember original size of log file.  */
  offset = __lseek64 (fd, 0, SEEK_END);
  const size_t utmp_size = last_entry_size (mode);
  if (offset % utmp_size != 0)
    {
      offset -= offset % utmp_size;
      __ftruncate64 (fd, offset);

      if (__lseek64 (fd, 0, SEEK_END) < 0)
	goto unlock_return;
    }

  /* Write the entry.  If we can't write all the bytes, reset the file
     size back to the original size.  That way, no partial entries
     will remain.  */
  if (__write_nocancel (fd, utmp, utmp_size) != utmp_size)
    {
      __ftruncate64 (fd, offset);
      goto unlock_return;
    }

  result = 0;

unlock_return:
  file_unlock (fd);

  /* Close WTMP file.  */
  __close_nocancel_nostatus (fd);

  return result;
}

void
__libc_setutent (void)
{
  internal_setutent (UTMP_TIME64);
}

void
__libc_setutent32 (void)
{
  internal_setutent (UTMP_TIME32);
}

int
__libc_getutent_r (struct utmp *buffer, struct utmp **result)
{
  int r = internal_getutent_r (UTMP_TIME64, buffer);
  *result = r == 0 ? buffer : NULL;
  return r;
}

/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
int
__libc_getutid_r (const struct utmp *id, struct utmp *buffer,
		  struct utmp **result)
{
  int r = internal_getutid_r (UTMP_TIME64, id->ut_type, id->ut_id,
			      id->ut_line, buffer);
  *result = r == 0? buffer : NULL;
  return r;
}

/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
int
__libc_getutline_r (const struct utmp *line, struct utmp *buffer,
		    struct utmp **result)
{
  int r = internal_getutline_r (UTMP_TIME64, line->ut_line, buffer);
  *result = r == 0 ? buffer : NULL;
  return r;
}

struct utmp *
__libc_pututline (const struct utmp *line)
{
  return internal_pututline (UTMP_TIME64, line->ut_type, line->ut_id,
			     line->ut_line, line)
	 ? (struct utmp *) line : NULL;
}

void
__libc_endutent (void)
{
  if (file_fd >= 0)
    {
      __close_nocancel_nostatus (file_fd);
      file_fd = -1;
    }
}

int
__libc_updwtmp (const char *file, const struct utmp *utmp)
{
  return internal_updwtmp (UTMP_TIME64, file, utmp);
}

#if SHLIB_COMPAT(libc, GLIBC_2_0, UTMP_COMPAT_BASE)
int
__libc_getutent32_r (struct utmp32 *buffer, struct utmp32 **result)
{
  int r = internal_getutent_r (UTMP_TIME32, buffer);
  *result = r == 0 ? buffer : NULL;
  return r;
}

int
__libc_getutid32_r (const struct utmp32 *id, struct utmp32 *buffer,
		    struct utmp32 **result)
{
  int r = internal_getutid_r (UTMP_TIME32, id->ut_type, id->ut_id,
			      id->ut_line, buffer);
  *result = r == 0 ? buffer : NULL;
  return r;
}

int
__libc_getutline32_r (const struct utmp32 *line, struct utmp32 *buffer,
		      struct utmp32 **result)
{
  int r = internal_getutline_r (UTMP_TIME32, line->ut_line, buffer);
  *result = r == 0 ? buffer : NULL;
  return r;
}

struct utmp32 *
__libc_pututline32 (const struct utmp32 *line)
{
  return internal_pututline (UTMP_TIME32, line->ut_type, line->ut_id,
			     line->ut_line, line)
	 ? (struct utmp32 *) line : NULL;
}

int
__libc_updwtmp32 (const char *file, const struct utmp32 *utmp)
{
  return internal_updwtmp (UTMP_TIME32, file, utmp);
}
#endif
