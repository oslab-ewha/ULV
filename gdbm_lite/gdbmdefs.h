#include "systems.h"
#include "gdbmconst.h"
#include "gdbm.h"

#define _(s) s
#define N_(s) s

/* The width in bits of the integer type or expression T. */
#define TYPE_WIDTH(t) (sizeof (t) * CHAR_BIT)

#define SIGNED_TYPE_MAXIMUM(t) \
  ((t) ((((t) 1 << (TYPE_WIDTH (t) - 2)) - 1) * 2 + 1))

/* Maximum value for off_t */
#define OFF_T_MAX SIGNED_TYPE_MAXIMUM (off_t)

/* Return true if A can be added to B without integer overflow */
static inline off_t
off_t_sum_ok (off_t a, off_t b)
{
  return OFF_T_MAX - a >= b;
}

typedef struct
{
  int   av_size;                /* The size of the available block. */
  off_t  av_adr;                /* The file address of the available block. */
} avail_elem;

/* This is the actual table. The in-memory images of the avail blocks are
   allocated by malloc using a calculated size.  */
typedef struct
{
  int   size;             /* The number of avail elements in the table.*/
  int   count;            /* The number of entries in the table. */
  off_t next_block;       /* The file address of the next avail block. */
  avail_elem av_table[1]; /* The table.  Make it look like an array.  */
} avail_block;

typedef struct
{
  int   hash_value;       /* The complete 31 bit value. */
  char  key_start[SMALL]; /* Up to the first SMALL bytes of the key.  */
  off_t data_pointer;     /* The file address of the key record. The
			     data record directly follows the key.  */
  int   key_size;         /* Size of key data in the file. */
  int   data_size;        /* Size of associated data in the file. */
} bucket_element;

extern int gdbm_bucket_element_valid_p (GDBM_FILE dbf, int elem_loc);

typedef struct
{
  int   av_count;            /* The number of bucket_avail entries. */
  avail_elem bucket_avail[BUCKET_AVAIL];  /* Distributed avail. */
  int   bucket_bits;         /* The number of bits used to get here. */
  int   count;               /* The number of element buckets full. */
  bucket_element h_table[1]; /* The table.  Make it look like an array.*/
} hash_bucket;

typedef struct
{
  int     hash_val;
  int     data_size;
  int     key_size;
  char    *dptr;
  size_t  dsize;
  int     elem_loc;
} data_cache_elem;

typedef struct
{
  hash_bucket *   ca_bucket;
  off_t           ca_adr;
  char            ca_changed;   /* Data in the bucket changed. */
  data_cache_elem ca_data;
} cache_elem;

/* Return true if avail_block is valid */
static inline int
gdbm_avail_block_valid_p (avail_block const *av)
{
  return (av->size > 1 && av->count >= 0 && av->count <= av->size);
}

/* Return true if both AV and the size of AV->av_table are valid.
   See comment to this function in gdbmopen.c */
extern int gdbm_avail_table_valid_p (GDBM_FILE dbf, avail_elem *av, int count);

typedef struct
{
  int   header_magic;  /* Version of file. */
  int   block_size;    /* The optimal i/o blocksize from stat. */
  off_t dir;           /* File address of hash directory table. */
  int   dir_size;      /* Size in bytes of the table.  */
  int   dir_bits;      /* The number of address bits used in the table.*/
  int   bucket_size;   /* Size in bytes of a hash bucket struct. */
  int   bucket_elems;  /* Number of elements in a hash bucket. */
  off_t next_block;    /* The next unallocated block address. */
  avail_block avail;   /* This must be last because of the pseudo
                          array in avail.  This avail grows to fill
                          the entire block. */
} gdbm_file_header;

struct gdbm_file_info
{
  /* Global variables and pointers to dynamic variables used by gdbm.  */

  /* The file name. */
  char *name;

  /* The reader/writer status. */
  unsigned read_write :2;

  /* Fast_write is set to 1 if no fsyncs are to be done. */
  unsigned fast_write :1;

  /* Central_free is set if all free blocks are kept in the header. */
  unsigned central_free :1;

  /* Coalesce_blocks is set if we should try to merge free blocks. */
  unsigned coalesce_blocks :1;

  /* Whether or not we should do file locking ourselves. */
  unsigned file_locking :1;

  /* Whether or not we're allowing mmap() use. */
  unsigned memory_mapping :1;

  /* Whether the database was open with GDBM_CLOEXEC flag */
  unsigned cloexec :1;

  /* Last error was fatal, the database needs recovery */
  unsigned need_recovery :1;
  
  /* Last GDBM error number */
  gdbm_error last_error;
  /* Last system error number */
  int last_syserror;
  /* Last formatted error */
  char *last_errstr;
  
  /* Type of file locking in use. */
  enum { LOCKING_NONE = 0, LOCKING_FLOCK, LOCKING_LOCKF,
	 LOCKING_FCNTL } lock_type;

  /* The fatal error handling routine. */
  void (*fatal_err) (const char *);

  /* The gdbm file descriptor which is set in gdbm_open.  */
  int desc;

  /* The file header holds information about the database. */
  gdbm_file_header *header;
  
  /* The hash table directory from extendable hashing.  See Fagin et al, 
     ACM Trans on Database Systems, Vol 4, No 3. Sept 1979, 315-344 */
  off_t *dir;

  /* The bucket cache. */
  cache_elem *bucket_cache;
  size_t cache_size;
  size_t last_read;

  /* Points to the current hash bucket in the cache. */
  hash_bucket *bucket;

  /* The directory entry used to get the current hash bucket. */
  int bucket_dir;

  /* Pointer to the current bucket's cache entry. */
  cache_elem *cache_entry;

  /* Bookkeeping of things that need to be written back at the
     end of an update. */
  unsigned header_changed :1;
  unsigned directory_changed :1;
  unsigned bucket_changed :1;
  unsigned second_changed :1;

  /* Mmap info */
  size_t mapped_size_max;/* Max. allowed value for mapped_size */
  void  *mapped_region;  /* Mapped region */
  size_t mapped_size;    /* Size of the region */
  off_t  mapped_pos;     /* Current offset in the region */
  off_t  mapped_off;     /* Position in the file where the region
			    begins */
};

#define GDBM_DIR_COUNT(db) ((db)->header->dir_size / sizeof (off_t))

#define SAVE_ERRNO(code)                        \
  do                                            \
    {                                           \
      int __ec = errno;                         \
      code;                                     \
      errno = __ec;                             \
    }                                           \
  while (0)                                     \

/* Return immediately if the database needs recovery */	
#define GDBM_ASSERT_CONSISTENCY(dbf, onerr)			\
  do								\
    {								\
      if (dbf->need_recovery)					\
	{							\
          GDBM_SET_ERRNO (dbf, GDBM_NEED_RECOVERY, TRUE);	\
	  return onerr;						\
	}							\
    }								\
  while (0)

# define GDBM_DEBUG(flags, fmt, ...)
# define GDBM_DEBUG_DATUM(flags, dat, fmt, ...)
# define GDBM_SET_ERRNO2(dbf, ec, fatal, m) gdbm_set_errno (dbf, ec, fatal)

# define GDBM_SET_ERRNO(dbf, ec, fatal) GDBM_SET_ERRNO2 (dbf, ec, fatal, 0)

#include "proto.h"
