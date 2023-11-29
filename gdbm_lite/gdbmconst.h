#define TRUE    1
#define FALSE   0

#define GDBM_OMAGIC		0x13579ace	/* Original magic number. */
#define GDBM_MAGIC32		0x13579acd	/* New 32bit magic number. */
#define GDBM_MAGIC64		0x13579acf	/* New 64bit magic number. */

#define GDBM_OMAGIC_SWAP	0xce9a5713	/* OMAGIC swapped. */
#define GDBM_MAGIC32_SWAP	0xcd9a5713	/* MAGIC32 swapped. */
#define GDBM_MAGIC64_SWAP	0xcf9a5713	/* MAGIC64 swapped. */

#define IGNORE_SIZE 4

#define SMALL    4

#define BUCKET_AVAIL 6

/* Size of a hash value, in bits */
#define GDBM_HASH_BITS 31

/* Minimal acceptable block size */
#define GDBM_MIN_BLOCK_SIZE 512

/* The size of the bucket cache. */
#define DEFAULT_CACHESIZE  100

# define SIZE_T_MAX ((size_t)-1)
