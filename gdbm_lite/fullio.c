#include "autoconf.h"
#include "gdbmdefs.h"

/* Read exactly SIZE bytes of data into BUFFER.  Return value is 0 on
   success, and -1 on error.  In the latter case, gdbm_errno is set to
   GDBM_FILE_EOF, if not enough data is available, and to
   GDBM_FILE_READ_ERROR, if a read error occurs. */
int
_gdbm_full_read (GDBM_FILE dbf, void *buffer, size_t size)
{
  char *ptr = buffer;
  while (size)
    {
      ssize_t rdbytes = gdbm_file_read (dbf, ptr, size);
      if (rdbytes == -1)
	{
	  if (errno == EINTR)
	    continue;
	  if (gdbm_last_errno (dbf) == GDBM_NO_ERROR)
	    GDBM_SET_ERRNO (dbf, GDBM_FILE_READ_ERROR, FALSE);
	  return -1;
	}
      if (rdbytes == 0)
	{
	  GDBM_SET_ERRNO (dbf, GDBM_FILE_EOF, FALSE);
	  return -1;
	}
      ptr += rdbytes;
      size -= rdbytes;
    }
  return 0;
}

/* Write exactly SIZE bytes of data from BUFFER tp DBF.  Return 0 on
   success, and -1 (setting gdbm_errno to GDBM_FILE_READ_ERROR) on error. */
int
_gdbm_full_write (GDBM_FILE dbf, void *buffer, size_t size)
{
  char *ptr = buffer;
  while (size)
    {
      ssize_t wrbytes = gdbm_file_write (dbf, ptr, size);
      if (wrbytes == -1)
	{
	  if (errno == EINTR)
	    continue;
	  if (gdbm_last_errno (dbf) == GDBM_NO_ERROR)
	    GDBM_SET_ERRNO (dbf, GDBM_FILE_WRITE_ERROR, TRUE);
	  return -1;
	}
      if (wrbytes == 0)
	{
	  errno = ENOSPC;
	  GDBM_SET_ERRNO (dbf, GDBM_FILE_WRITE_ERROR, TRUE);
	  return -1;
	}
      ptr += wrbytes;
      size -= wrbytes;
    }
  return 0;
}

/* Grow the disk file of DBF to SIZE bytes in length. Fill the
   newly allocated space with zeros. */
int
_gdbm_file_extend (GDBM_FILE dbf, off_t size)
{
  size_t page_size = GDBM_PAGESIZE;
  char *buf;
  off_t file_end;

  file_end = lseek (dbf->desc, 0, SEEK_END);
  if (!file_end)
    {
      GDBM_SET_ERRNO (dbf, GDBM_FILE_SEEK_ERROR, FALSE);
      return -1;
    }
  size -= file_end;
  if (size > 0)
    {
      if (size < page_size)
	page_size = size;
      buf = calloc (1, page_size);
      if (!buf)
	{
	  GDBM_SET_ERRNO (dbf, GDBM_MALLOC_ERROR, FALSE);
	  return -1;
	}

      while (size)
	{
	  ssize_t n = write (dbf->desc, buf,
			     size < page_size ? size : page_size);
	  if (n <= 0)
	    {
	      GDBM_SET_ERRNO (dbf, GDBM_FILE_WRITE_ERROR, TRUE);
	      break;
	    }
	  size -= n;
	}
      free (buf);
      if (size)
	return -1;
    }
  return 0;
}
