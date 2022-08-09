#include "ulv_dyntab.h"
#include "ulv_types.h"
#include "ulv_test.h"

typedef struct {
	int	x1, x2;
} entry_t;

int
main(int argc, char *argv[])
{
	ulv_dyntab_t	dyntab;
	entry_t	*entry0, *entry1, *entry2, *entry3;

	ulv_dyntab_init(&dyntab, sizeof(entry_t), 3);
	entry0 = (entry_t *)ulv_dyntab_assign(&dyntab);
	if (ULV_DYNTAB_ENTRY_IDX(entry0) != 0)
		FAIL("unexpected index of the first entry: %d", ULV_DYNTAB_ENTRY_IDX(entry0));
	entry0->x1 = 0;
	entry0->x2 = 100;

	entry1 = (entry_t *)ulv_dyntab_assign(&dyntab);
	if (ULV_DYNTAB_ENTRY_IDX(entry1) != 1)
		FAIL("unexpected index of the second entry: %d", ULV_DYNTAB_ENTRY_IDX(entry1));

	entry2 = (entry_t *)ulv_dyntab_assign(&dyntab);
	if (ULV_DYNTAB_ENTRY_IDX(entry2) != 2)
		FAIL("unexpected index of the third entry: %d", ULV_DYNTAB_ENTRY_IDX(entry2));
	entry2->x1 = 2;
	entry2->x2 = 102;

	ulv_dyntab_release(&dyntab, entry1);

	entry1 = (entry_t *)ulv_dyntab_assign(&dyntab);

	if (ULV_DYNTAB_ENTRY_IDX(entry1) != 1)
		FAIL("unexpected index of realloced entry: %d", ULV_DYNTAB_ENTRY_IDX(entry1));

	entry3 = (entry_t *)ulv_dyntab_assign(&dyntab);
	if (ULV_DYNTAB_ENTRY_IDX(entry3) != 3)
		FAIL("unexpected index of the fouth entry: %d", ULV_DYNTAB_ENTRY_IDX(entry3));
	entry3->x1 = 3;
	entry3->x2 = 103;

	entry0 = (entry_t *)ulv_dyntab_get(&dyntab, 0);
	if (entry0 == NULL || entry0->x2 != 100)
		FAIL("unexpected value of the first acquired entry: %d", entry0 == NULL ? -1: entry0->x2);

	entry2 = (entry_t *)ulv_dyntab_get(&dyntab, 2);
	if (entry2 == NULL || entry2->x2 != 102)
		FAIL("unexpected value of the third acuiqred entry: %d", entry2 == NULL ? -1: entry2->x2);

	ulv_dyntab_fini(&dyntab);

	PASS("dynamic table OK");
}
