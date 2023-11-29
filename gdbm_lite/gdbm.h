#ifndef _GDBM_H_
# define _GDBM_H_

# define GDBM_READER	0	/* A reader. */
# define GDBM_WRITER	1	/* A writer. */
# define GDBM_WRCREAT	2	/* A writer.  Create the db if needed. */
# define GDBM_NEWDB	3	/* A writer.  Always create a new db. */
# define GDBM_OPENMASK	7	/* Mask for the above. */

# define GDBM_FAST	0x010	/* Write fast! => No fsyncs.  OBSOLETE. */
# define GDBM_SYNC	0x020	/* Sync operations to the disk. */
# define GDBM_NOLOCK	0x040	/* Don't do file locking operations. */
# define GDBM_NOMMAP	0x080	/* Don't use mmap(). */
# define GDBM_CLOEXEC   0x100   /* Close the underlying fd on exec(3) */
# define GDBM_BSEXACT   0x200   /* Don't adjust block_size. Bail out with
				   GDBM_BLOCK_SIZE_ERROR error if unable to
				   set it. */  
# define GDBM_CLOERROR  0x400   /* Only for gdbm_fd_open: close fd on error. */

# define GDBM_INSERT	0	/* Never replace old data with new. */
# define GDBM_REPLACE	1	/* Always replace old data with new. */

typedef int gdbm_error;
extern int *gdbm_errno_location (void);
#define gdbm_errno (*gdbm_errno_location ())

/* The data and key structure. */
typedef struct
{
  char *dptr;
  int   dsize;
} datum;

typedef struct gdbm_file_info *GDBM_FILE;

extern int gdbm_close (GDBM_FILE);
extern int gdbm_store (GDBM_FILE, datum, datum, int);
extern datum gdbm_fetch (GDBM_FILE, datum);

extern gdbm_error gdbm_last_errno (GDBM_FILE dbf);
extern int gdbm_last_syserr (GDBM_FILE dbf);
extern void gdbm_set_errno (GDBM_FILE dbf, gdbm_error ec, int fatal);
extern void gdbm_clear_error (GDBM_FILE dbf);

extern const char *gdbm_strerror (gdbm_error);
extern const char *gdbm_db_strerror (GDBM_FILE dbf);

# define GDBM_NO_ERROR		         0
# define GDBM_MALLOC_ERROR	         1
# define GDBM_BLOCK_SIZE_ERROR	         2
# define GDBM_FILE_OPEN_ERROR	         3
# define GDBM_FILE_WRITE_ERROR	         4
# define GDBM_FILE_SEEK_ERROR	         5
# define GDBM_FILE_READ_ERROR	         6
# define GDBM_BAD_MAGIC_NUMBER	         7
# define GDBM_EMPTY_DATABASE	         8
# define GDBM_CANT_BE_READER	         9
# define GDBM_CANT_BE_WRITER	        10
# define GDBM_READER_CANT_DELETE	11
# define GDBM_READER_CANT_STORE	        12
# define GDBM_READER_CANT_REORGANIZE	13
# define GDBM_UNKNOWN_ERROR	        14
# define GDBM_ITEM_NOT_FOUND	        15
# define GDBM_REORGANIZE_FAILED	        16
# define GDBM_CANNOT_REPLACE	        17
# define GDBM_ILLEGAL_DATA	        18
# define GDBM_OPT_ALREADY_SET	        19
# define GDBM_OPT_ILLEGAL           	20
# define GDBM_BYTE_SWAPPED	        21
# define GDBM_BAD_FILE_OFFSET	        22
# define GDBM_BAD_OPEN_FLAGS	        23
# define GDBM_FILE_STAT_ERROR           24
# define GDBM_FILE_EOF                  25
# define GDBM_NO_DBNAME                 26
# define GDBM_ERR_FILE_OWNER            27
# define GDBM_ERR_FILE_MODE             28
# define GDBM_NEED_RECOVERY             29
# define GDBM_BACKUP_FAILED             30
# define GDBM_DIR_OVERFLOW              31
# define GDBM_BAD_BUCKET                32
# define GDBM_BAD_HEADER                33
# define GDBM_BAD_AVAIL                 34
# define GDBM_BAD_HASH_TABLE            35
# define GDBM_BAD_DIR_ENTRY             36
# define GDBM_FILE_CLOSE_ERROR          37  
# define GDBM_FILE_SYNC_ERROR           38
# define GDBM_FILE_TRUNCATE_ERROR       39

# define _GDBM_MIN_ERRNO	0
# define _GDBM_MAX_ERRNO        GDBM_FILE_TRUNCATE_ERROR

extern const char * const gdbm_errlist[];

#endif
