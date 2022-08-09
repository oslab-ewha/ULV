#ifndef _ULV_DYNTAB_H_
#define _ULV_DYNTAB_H_

typedef struct {
	unsigned	idx_avail, n_alloced;
	unsigned	size_entry;
	unsigned	alloc_chunk;
	void	*entries;
} ulv_dyntab_t;

#define ULV_DYNTAB_ENTRY_IDX(entry)	(*((unsigned *)(entry) - 1) - 1)

void ulv_dyntab_init(ulv_dyntab_t *pdyntab, unsigned size_entry, unsigned alloc_chunk);
void ulv_dyntab_fini(ulv_dyntab_t *pdyntab);
void *ulv_dyntab_assign(ulv_dyntab_t *pdyntab);
void *ulv_dyntab_get(ulv_dyntab_t *pdyntab, unsigned idx);
void ulv_dyntab_release(ulv_dyntab_t *pdyntab, void *entry);

#endif
