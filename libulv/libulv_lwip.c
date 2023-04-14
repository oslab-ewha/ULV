#include "ulv_build.h"

extern _BUILD_WEAK void lwip_socket(void);
extern _BUILD_WEAK void lwip_connect(void);

__attribute__((used))
static void
dummy(void)
{
	lwip_socket();
	lwip_connect();
}
