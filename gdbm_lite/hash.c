#include "autoconf.h"

#include "gdbmdefs.h"

/* This hash function computes a GDBM_HASH_BITS-bit value.  The value is used
   to index the hash directory using the top n bits.  It is also used in a
   hash bucket to find the home position of the element by taking the value
   modulo the bucket hash table size. */

int
_gdbm_hash (datum key)
{
  unsigned int value;	/* Used to compute the hash value.  */
  int   index;		/* Used to cycle through random values. */

  /* Set the initial value from key. */
  value = 0x238F13AF * key.dsize;
  for (index = 0; index < key.dsize; index++)
    value = (value + (key.dptr[index] << (index*5 % 24))) & 0x7FFFFFFF;

  value = (1103515243 * value + 12345) & 0x7FFFFFFF;  

  /* Return the value. */
  return((int) value);
}

int
_gdbm_bucket_dir (GDBM_FILE dbf, int hash)
{
  return hash >> (GDBM_HASH_BITS - dbf->header->dir_bits);
}

void
_gdbm_hash_key (GDBM_FILE dbf, datum key, int *hash, int *bucket, int *offset)
{
  int hashval = _gdbm_hash (key);
  *hash = hashval;
  *bucket = _gdbm_bucket_dir (dbf, hashval);
  *offset = hashval % dbf->header->bucket_elems;
}
