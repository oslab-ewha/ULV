#include "ulv_dyntab.h"
#include "ulv_malloc.h"
#include "ulv_assert.h"

#define ENTRY_IDX_SIZE(pdyntab)		((pdyntab)->size_entry + sizeof(unsigned))

#define DYNTAB_IDX(entry)	*((unsigned *)(entry) - 1)
#define DYNTAB_ENTRY(pdyntab, idx)	((char *)(pdyntab)->entries + ENTRY_IDX_SIZE(pdyntab) * idx + sizeof(unsigned))
#define DYNTAB_NEXT(pdyntab, entry)	((char *)(entry) + ENTRY_IDX_SIZE(pdyntab))

static unsigned
find_next_idx_avail(ulv_dyntab_t *pdyntab)
{
	void	*entry;
	int	i;

	for (i = pdyntab->idx_avail + 1, entry = DYNTAB_ENTRY(pdyntab, i); i < pdyntab->n_alloced;
	     i++, entry = DYNTAB_NEXT(pdyntab, entry)) {
		if (DYNTAB_IDX(entry) == 0)
			return i;
	}

	return i;
}

static void
augment_entries(ulv_dyntab_t *pdyntab)
{
	void	*entry;
	int	i;

	pdyntab->n_alloced += pdyntab->alloc_chunk;
	pdyntab->entries = ulv_realloc(pdyntab->entries, ENTRY_IDX_SIZE(pdyntab) * pdyntab->n_alloced);
	if (pdyntab->entries == NULL)
		ULV_PANIC("dynamic table: out of memory");
	for (i = pdyntab->n_alloced - pdyntab->alloc_chunk, entry = DYNTAB_ENTRY(pdyntab, i); i < pdyntab->n_alloced;
	     i++, entry = DYNTAB_NEXT(pdyntab, entry))
		DYNTAB_IDX(entry) = 0;
}

void *
ulv_dyntab_assign(ulv_dyntab_t *pdyntab)
{
	void	*entry;
	unsigned	idx;

	idx = pdyntab->idx_avail;
	pdyntab->idx_avail = find_next_idx_avail(pdyntab);
	if (pdyntab->idx_avail == pdyntab->n_alloced)
		augment_entries(pdyntab);
	entry = DYNTAB_ENTRY(pdyntab, idx);
	DYNTAB_IDX(entry) = idx + 1;

	return entry;
}

void
ulv_dyntab_release(ulv_dyntab_t *pdyntab, void *entry)
{
	unsigned	idx = DYNTAB_IDX(entry);

	if (idx - 1 < pdyntab->idx_avail)
		pdyntab->idx_avail = idx - 1;
	DYNTAB_IDX(entry) = 0;
}

void *
ulv_dyntab_get(ulv_dyntab_t *pdyntab, unsigned idx)
{
	void	*entry;

	if (idx >= pdyntab->n_alloced)
		return NULL;
	entry = DYNTAB_ENTRY(pdyntab, idx);
	if (DYNTAB_IDX(entry) == idx + 1)
		return entry;
	return NULL;
}

void
ulv_dyntab_init(ulv_dyntab_t *pdyntab, unsigned size_entry, unsigned alloc_chunk)
{
	pdyntab->idx_avail = 0;
	pdyntab->n_alloced = 0;
	pdyntab->size_entry = size_entry;
	pdyntab->alloc_chunk = alloc_chunk;
	pdyntab->entries = NULL;

	augment_entries(pdyntab);
}

void
ulv_dyntab_fini(ulv_dyntab_t *pdyntab)
{
	ulv_free(pdyntab->entries);
}
