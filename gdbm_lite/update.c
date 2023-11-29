#include "autoconf.h"

#include "gdbmdefs.h"

static int
write_header (GDBM_FILE dbf)
{
  off_t file_pos;	/* Return value for lseek. */
  int rc;

  file_pos = gdbm_file_seek (dbf, 0L, SEEK_SET);
  if (file_pos != 0)
    {
      GDBM_SET_ERRNO2 (dbf, GDBM_FILE_SEEK_ERROR, TRUE, GDBM_DEBUG_STORE);
      _gdbm_fatal (dbf, _("lseek error"));
      return -1;
    }

  rc = _gdbm_full_write (dbf, dbf->header, dbf->header->block_size);
  
  if (rc)
    {
      GDBM_DEBUG (GDBM_DEBUG_STORE|GDBM_DEBUG_ERR,
		  "%s: error writing header: %s",
		  dbf->name, gdbm_db_strerror (dbf));	        
      return -1;
    }

  /* Sync the file if fast_write is FALSE. */
  if (dbf->fast_write == FALSE)
    gdbm_file_sync (dbf);

  return 0;
}

int
_gdbm_end_update (GDBM_FILE dbf)
{
  off_t file_pos;	/* Return value for lseek. */
  int rc;
  
  /* Write the current bucket. */
  if (dbf->bucket_changed && (dbf->cache_entry != NULL))
    {
      if (_gdbm_write_bucket (dbf, dbf->cache_entry))
	return -1;
      dbf->bucket_changed = FALSE;
    }

  /* Write the other changed buckets if there are any. */
  if (dbf->second_changed)
    {
      if (dbf->bucket_cache != NULL)
        {
          int index;

          for (index = 0; index < dbf->cache_size; index++)
	    {
	      if (dbf->bucket_cache[index].ca_changed)
		{
		  if (_gdbm_write_bucket (dbf, &dbf->bucket_cache[index]))
		    return -1;
		}
            }
        }
      dbf->second_changed = FALSE;
    }
  
  /* Write the directory. */
  if (dbf->directory_changed)
    {
      file_pos = gdbm_file_seek (dbf, dbf->header->dir, SEEK_SET);
      if (file_pos != dbf->header->dir)
	{
	  GDBM_SET_ERRNO2 (dbf, GDBM_FILE_SEEK_ERROR, TRUE, GDBM_DEBUG_STORE);
	  _gdbm_fatal (dbf, _("lseek error"));
	  return -1;
	}

      rc = _gdbm_full_write (dbf, dbf->dir, dbf->header->dir_size);
      if (rc)
	{
	  GDBM_DEBUG (GDBM_DEBUG_STORE|GDBM_DEBUG_ERR,
		      "%s: error writing directory: %s",
		      dbf->name, gdbm_db_strerror (dbf));	  	  
	  _gdbm_fatal (dbf, gdbm_db_strerror (dbf));
	  return -1;
	}

      dbf->directory_changed = FALSE;
      if (!dbf->header_changed && dbf->fast_write == FALSE)
	gdbm_file_sync (dbf);
    }

  /* Final write of the header. */
  if (dbf->header_changed)
    {
      if (write_header (dbf))
	return -1;
      if (_gdbm_file_extend (dbf, dbf->header->next_block))
	return -1;
      dbf->header_changed = FALSE;
    }

  return 0;
}

void
_gdbm_fatal (GDBM_FILE dbf, const char *val)
{
  if (dbf && dbf->fatal_err)
    {
      (*dbf->fatal_err) (val);
#if 0 //unikraft does not support
      exit (1);
#endif
    }
}
