#include "autoconf.h"

#include "gdbmdefs.h"

#include <errno.h>

void
_gdbm_unlock_file (GDBM_FILE dbf)
{
#if HAVE_FCNTL_LOCK
  struct flock fl;
#endif

  switch (dbf->lock_type)
    {
      case LOCKING_FLOCK:
#if HAVE_FLOCK
	flock (dbf->desc, LOCK_UN);
#endif
	break;

      case LOCKING_LOCKF:
#if HAVE_LOCKF
	lockf (dbf->desc, F_ULOCK, (off_t)0L);
#endif
	break;

      case LOCKING_FCNTL:
#if HAVE_FCNTL_LOCK
	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = fl.l_len = (off_t)0L;
	fcntl (dbf->desc, F_SETLK, &fl);
#endif
	break;

      case LOCKING_NONE:
        break;
    }

  dbf->lock_type = LOCKING_NONE;
}

int
_gdbm_lock_file (GDBM_FILE dbf)
{
#if HAVE_FCNTL_LOCK
  struct flock fl;
#endif
  int lock_val = -1;

#if HAVE_FLOCK
  if (dbf->read_write == GDBM_READER)
    lock_val = flock (dbf->desc, LOCK_SH + LOCK_NB);
  else
    lock_val = flock (dbf->desc, LOCK_EX + LOCK_NB);

  if ((lock_val == -1) && (errno == EWOULDBLOCK))
    {
      dbf->lock_type = LOCKING_NONE;
      return lock_val;
    }
  else if (lock_val != -1)
    {
      dbf->lock_type = LOCKING_FLOCK;
      return lock_val;
    }
#endif

#if HAVE_LOCKF
  /* Mask doesn't matter for lockf. */
  lock_val = lockf (dbf->desc, F_LOCK, (off_t)0L);
  if ((lock_val == -1) && (errno == EDEADLK))
    {
      dbf->lock_type = LOCKING_NONE;
      return lock_val;
    }
  else if (lock_val != -1)
    {
      dbf->lock_type = LOCKING_LOCKF;
      return lock_val;
    }
#endif

#if HAVE_FCNTL_LOCK
  /* If we're still here, try fcntl. */
  if (dbf->read_write == GDBM_READER)
    fl.l_type = F_RDLCK;
  else
    fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = (off_t)0L;
  lock_val = fcntl (dbf->desc, F_SETLK, &fl);

  if (lock_val != -1)
    dbf->lock_type = LOCKING_FCNTL;
#endif

  if (lock_val == -1)
    dbf->lock_type = LOCKING_NONE;
  return lock_val;
}
