#include "autoconf.h"

#include "gdbmdefs.h"

static GDBM_THREAD_LOCAL gdbm_error gdbm_errno_storage = GDBM_NO_ERROR;

int *
gdbm_errno_location (void)
{
  return &gdbm_errno_storage;
}

const char * const gdbm_errlist[_GDBM_MAX_ERRNO+1] = {
  [GDBM_NO_ERROR]               = N_("No error"),
  [GDBM_MALLOC_ERROR]           = N_("Malloc error"),
  [GDBM_BLOCK_SIZE_ERROR]       = N_("Block size error"),
  [GDBM_FILE_OPEN_ERROR]        = N_("File open error"),
  [GDBM_FILE_WRITE_ERROR]       = N_("File write error"),
  [GDBM_FILE_SEEK_ERROR]        = N_("File seek error"),
  [GDBM_FILE_READ_ERROR]        = N_("File read error"),
  [GDBM_BAD_MAGIC_NUMBER]       = N_("Bad magic number"),
  [GDBM_EMPTY_DATABASE]         = N_("Empty database"),
  [GDBM_CANT_BE_READER]         = N_("Can't be reader"),
  [GDBM_CANT_BE_WRITER]         = N_("Can't be writer"),
  [GDBM_READER_CANT_DELETE]     = N_("Reader can't delete"),
  [GDBM_READER_CANT_STORE]      = N_("Reader can't store"),
  [GDBM_READER_CANT_REORGANIZE] = N_("Reader can't reorganize"),
  [GDBM_UNKNOWN_ERROR]          = N_("Should not happen: unused error code"),
  [GDBM_ITEM_NOT_FOUND]         = N_("Item not found"),
  [GDBM_REORGANIZE_FAILED]      = N_("Reorganize failed"),
  [GDBM_CANNOT_REPLACE]         = N_("Cannot replace"),
  [GDBM_ILLEGAL_DATA]           = N_("Illegal data"),
  [GDBM_OPT_ALREADY_SET]        = N_("Option already set"),
  [GDBM_OPT_ILLEGAL]            = N_("Illegal option"),
  [GDBM_BYTE_SWAPPED]           = N_("Byte-swapped file"),
  [GDBM_BAD_FILE_OFFSET]        = N_("File header assumes wrong off_t size"),
  [GDBM_BAD_OPEN_FLAGS]         = N_("Bad file flags"),
  [GDBM_FILE_STAT_ERROR]        = N_("Cannot stat file"),
  [GDBM_FILE_EOF]               = N_("Unexpected end of file"),
  [GDBM_NO_DBNAME]              = N_("Database name not given"),
  [GDBM_ERR_FILE_OWNER]         = N_("Failed to restore file owner"),
  [GDBM_ERR_FILE_MODE]          = N_("Failed to restore file mode"),
  [GDBM_NEED_RECOVERY]          = N_("Database needs recovery"),
  [GDBM_BACKUP_FAILED]          = N_("Failed to create backup copy"),
  [GDBM_DIR_OVERFLOW]           = N_("Bucket directory overflow"),
  [GDBM_BAD_BUCKET]             = N_("Malformed bucket header"),
  [GDBM_BAD_HEADER]             = N_("Malformed database file header"),
  [GDBM_BAD_AVAIL]              = N_("Malformed avail_block"),
  [GDBM_BAD_HASH_TABLE]         = N_("Malformed hash table"),
  [GDBM_BAD_DIR_ENTRY]          = N_("Invalid directory entry"),
  [GDBM_FILE_CLOSE_ERROR]       = N_("Error closing file"),
  [GDBM_FILE_SYNC_ERROR]        = N_("Error synchronizing file"),
  [GDBM_FILE_TRUNCATE_ERROR]    = N_("Error truncating file")
};

const char *
gdbm_strerror (gdbm_error error)
{
  if (error < _GDBM_MIN_ERRNO || error > _GDBM_MAX_ERRNO)
    error = GDBM_UNKNOWN_ERROR;
  return gdbm_errlist[error];
}

char const *
gdbm_db_strerror (GDBM_FILE dbf)
{
  if (!dbf->last_errstr)
    {
      char const *errstr = gdbm_strerror (dbf->last_error);

      if (dbf->last_syserror)
	{
	  char const *syserrstr = strerror (dbf->last_syserror);
	  size_t len = strlen (errstr) + strlen (syserrstr) + 2;
	  dbf->last_errstr = malloc (len + 1);
	  if (!dbf->last_errstr)
	    return errstr;

	  strcpy (dbf->last_errstr, errstr);
#if 0 //unikraft does not support
	  strcat (dbf->last_errstr, ": ");
	  strcat (dbf->last_errstr, syserrstr);
#endif
	}
      else
	return errstr;
    }
  return dbf->last_errstr;
}

int const gdbm_syserr[_GDBM_MAX_ERRNO+1] = {
  [GDBM_FILE_OPEN_ERROR]        = 1,
  [GDBM_FILE_WRITE_ERROR]       = 1,
  [GDBM_FILE_SEEK_ERROR]        = 1,
  [GDBM_FILE_READ_ERROR]        = 1,
  [GDBM_FILE_STAT_ERROR]        = 1,
  [GDBM_BACKUP_FAILED]          = 1,
  [GDBM_FILE_CLOSE_ERROR]       = 1,
  [GDBM_FILE_SYNC_ERROR]        = 1,
  [GDBM_FILE_TRUNCATE_ERROR]    = 1
};

gdbm_error
gdbm_last_errno (GDBM_FILE dbf)
{
  if (!dbf)
    {
      errno = EINVAL;
      return -1;
    }
  return dbf->last_error;
}

int
gdbm_last_syserr (GDBM_FILE dbf)
{
  if (!dbf)
    {
      errno = EINVAL;
      return -1;
    }
  return dbf->last_syserror;
}

void
gdbm_clear_error (GDBM_FILE dbf)
{
  if (dbf)
    {
      dbf->last_error = GDBM_NO_ERROR;
      dbf->last_syserror = 0;
      free (dbf->last_errstr);
      dbf->last_errstr = NULL;
    }
}

void
gdbm_set_errno (GDBM_FILE dbf, gdbm_error ec, int fatal)
{
  if (dbf)
    {
      free (dbf->last_errstr);
      dbf->last_errstr = NULL;
      
      dbf->last_error = ec;
      if (gdbm_syserr[ec])
	dbf->last_syserror = errno;
      else
	dbf->last_syserror = 0;
      dbf->need_recovery = fatal;
    }
  gdbm_errno = ec;
}
