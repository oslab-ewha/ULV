void _gdbm_new_bucket	(GDBM_FILE, hash_bucket *, int);
int _gdbm_get_bucket	(GDBM_FILE, int);
int _gdbm_read_bucket_at (GDBM_FILE dbf, off_t off, hash_bucket *bucket,
			  size_t size);

int _gdbm_split_bucket (GDBM_FILE, int);
int _gdbm_write_bucket (GDBM_FILE, cache_elem *);

/* From falloc.c */
off_t _gdbm_alloc       (GDBM_FILE, int);
int  _gdbm_free         (GDBM_FILE, off_t, int);
void _gdbm_put_av_elem  (avail_elem, avail_elem [], int *, int);

/* From findkey.c */
char *_gdbm_read_entry  (GDBM_FILE, int);
int _gdbm_findkey       (GDBM_FILE, datum, char **, int *);

/* From hash.c */
int _gdbm_hash (datum);
void _gdbm_hash_key (GDBM_FILE dbf, datum key, int *hash, int *bucket,
		     int *offset);
int _gdbm_bucket_dir (GDBM_FILE dbf, int hash);

int _gdbm_end_update   (GDBM_FILE);
void _gdbm_fatal	(GDBM_FILE, const char *);

/* From gdbmopen.c */
int _gdbm_init_cache	(GDBM_FILE, size_t);
void _gdbm_cache_entry_invalidate (GDBM_FILE, int);

int gdbm_avail_block_validate (GDBM_FILE dbf, avail_block *avblk);
int gdbm_bucket_avail_table_validate (GDBM_FILE dbf, hash_bucket *bucket);

void _gdbm_unlock_file	(GDBM_FILE);
int _gdbm_lock_file	(GDBM_FILE);

int _gdbm_full_read (GDBM_FILE, void *, size_t);
int _gdbm_full_write (GDBM_FILE, void *, size_t);
int _gdbm_file_extend (GDBM_FILE dbf, off_t size);

/* I/O functions */
static inline ssize_t
gdbm_file_read (GDBM_FILE dbf, void *buf, size_t size)
{
#if HAVE_MMAP
  return _gdbm_mapped_read (dbf, buf, size);
#else
  return read (dbf->desc, buf, size);
#endif
}

static inline ssize_t
gdbm_file_write (GDBM_FILE dbf, void *buf, size_t size)
{
#if HAVE_MMAP
  return _gdbm_mapped_write (dbf, buf, size);
#else
  return write (dbf->desc, buf, size);
#endif
}

static inline off_t
gdbm_file_seek (GDBM_FILE dbf, off_t off, int whence)
{
#if HAVE_MMAP
  return _gdbm_mapped_lseek (dbf, off, whence);
#else
  return lseek (dbf->desc, off, whence);
#endif
}

static inline int
gdbm_file_sync (GDBM_FILE dbf)
{
#if HAVE_MMAP
  return _gdbm_mapped_sync (dbf);
#elif HAVE_FSYNC
  if (fsync (dbf->desc))
    {
      GDBM_SET_ERRNO (dbf, GDBM_FILE_SYNC_ERROR, TRUE);
      return 1;
    }
#else
  sync ();
  sync ();
#endif
  return 0;
}
