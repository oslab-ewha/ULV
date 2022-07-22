#include "uc_atomic.h"
#include "uc_assert.h"
#include "uc_syscalls.h"

#define NULL	((void *)0)

typedef struct chunk {
	int	size;
	int	next;
} chunk_t;

static int	locked;
static void	*mapped;
static unsigned long	mapped_size;
static chunk_t *chunks_free;

#define MAPPED_ALLOCSIZE	(1024 * 1024)

#define MALLOC_SIZE(size)	((size) + sizeof(int))
#define CHUNK_BY_OFFSET(p, off)	((chunk_t *)((char *)(p) + off))
#define CHUNK_NEXT(chunk)	(((chunk)->next == 0) ? NULL : CHUNK_BY_OFFSET(chunk, (chunk)->next))
#define CHUNK_TO_MEM(chunk)	((void *)((char *)(chunk) + sizeof(int)))
#define MEM_TO_CHUNK(mem)	CHUNK_BY_OFFSET(mem, - sizeof(int))
#define CHUNKS_OFFSET(c2, c1)	((char *)(c2) - (char *)(c1))

static chunk_t *
get_last_free_chunk(void)
{
	chunk_t	*chunk, *prev = NULL;

	for (chunk = chunks_free; chunk; chunk = CHUNK_NEXT(chunk))
		prev = chunk;
	return prev;
}

static chunk_t *
get_prev_free_chunk(chunk_t *chunk)
{
	chunk_t	*chunk_free, *prev = NULL;

	for (chunk_free = chunks_free; chunk_free; chunk_free = CHUNK_NEXT(chunk_free)) {
		if (chunk < chunk_free)
			break;
		prev = chunk_free;
	}
	return prev;
}

static void
alloc_free_chunks(void)
{
	void	*mapped_old = mapped;
	chunk_t	*chunk_new;

	mapped_size += MAPPED_ALLOCSIZE;

	mapped = sys_mmap(mapped_old, mapped_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mapped == MAP_FAILED)
		UC_ABORT();
	if (mapped_old == NULL)
		chunk_new = CHUNK_BY_OFFSET(mapped, 0);
	else
		chunk_new = CHUNK_BY_OFFSET(mapped_old, MAPPED_ALLOCSIZE);

	chunk_new->size = MAPPED_ALLOCSIZE;
	chunk_new->next = 0;

	if (chunks_free == NULL)
		chunks_free = chunk_new;
	else {
		chunk_t	*chunk_last = get_last_free_chunk();
		chunk_last->next = CHUNKS_OFFSET(chunk_new, chunk_last);
	}
}

static chunk_t *
find_free_chunk(unsigned size)
{
	chunk_t	*chunk, *prev = NULL;

	UC_ASSERT(chunks_free != NULL);

	for (chunk = chunks_free; chunk; chunk = CHUNK_NEXT(chunk)) {
		if (chunk->size >= MALLOC_SIZE(size)) {
			chunk_t	*chunk_next = CHUNK_NEXT(chunk);
			chunk_t	*chunk_remain = NULL;

			if (chunk->size > MALLOC_SIZE(size) + sizeof(chunk_t)) {
				if (prev)
					prev->next += MALLOC_SIZE(size);
				chunk_remain = CHUNK_BY_OFFSET(chunk, MALLOC_SIZE(size));
				chunk_remain->size = chunk->size - MALLOC_SIZE(size);
				if (chunk->next)
					chunk_remain->next = chunk->next - MALLOC_SIZE(size);
				else
					chunk_remain->next = 0;
				chunk->size = MALLOC_SIZE(size);
			}
			if (chunks_free == chunk) {
				if (chunk_remain)
					chunks_free = chunk_remain;
				else
					chunks_free = chunk_next;
			}
			return chunk;
		}
		prev = chunk;
	}
	return NULL;
}

void *
uc_malloc(unsigned size)
{
	chunk_t	*chunk;

	if (MALLOC_SIZE(size) > MAPPED_ALLOCSIZE)
		UC_ABORT();

	if (size == 0)
		return NULL;

	uc_spin_lock(&locked);

	if (chunks_free == NULL)
		alloc_free_chunks();

	chunk = find_free_chunk(size);
	if (chunk == NULL) {
		alloc_free_chunks();
		chunk = find_free_chunk(size);
		if (chunk == NULL)
			UC_ABORT();
	}

	uc_spin_unlock(&locked);

	if (chunk)
		return CHUNK_TO_MEM(chunk);
	/* Never reach */
	return NULL;
}

static inline void
merge_chunks(chunk_t *chunk1, chunk_t *chunk2)
{
	chunk1->size += chunk2->size;
	if (chunk2->next == 0)
		chunk1->next = 0;
	else
		chunk1->next += chunk2->next;
}

void
uc_free(void *ptr)
{
	chunk_t	*chunk;

	chunk = MEM_TO_CHUNK(ptr);

	uc_spin_lock(&locked);

	if (chunks_free == NULL) {
		chunks_free = chunk;
		chunk->next = 0;
	}
	else {
		chunk_t	*chunk_prev;

		chunk_prev = get_prev_free_chunk(chunk);
		if (chunk_prev == NULL) {
			if (CHUNK_BY_OFFSET(chunk, chunk->size) == chunks_free) {
				if (chunks_free->next == 0)
					chunk->next = 0;
				else
					chunk->next = chunk->size + chunks_free->next;
				chunk->size += chunks_free->size;
			}
			else
				chunk->next = CHUNKS_OFFSET(chunks_free, chunk);
			chunks_free = chunk;
		}
		else {
			if (chunk_prev->next != 0) {
				chunk_t	*chunk_next = CHUNK_NEXT(chunk_prev);

				if (CHUNK_BY_OFFSET(chunk_prev, chunk_prev->size) == chunk) {
					chunk_prev->size += chunk->size;
					if (chunk_prev->next == chunk_prev->size)
						merge_chunks(chunk_prev, chunk_next);
				}
				else {
					chunk_prev->next = CHUNKS_OFFSET(chunk, chunk_prev);
					chunk->next = CHUNKS_OFFSET(chunk_next, chunk);
					if (chunk->next == chunk->size)
						merge_chunks(chunk, chunk_next);
				}
			}
			else {
				if (CHUNK_BY_OFFSET(chunk_prev, chunk_prev->size) == chunk)
					chunk_prev->size += chunk->size;
				else {
					chunk_prev->next = CHUNKS_OFFSET(chunk, chunk_prev);
					chunk->next = 0;
				}
			}
		}
	}
	uc_spin_unlock(&locked);
}
