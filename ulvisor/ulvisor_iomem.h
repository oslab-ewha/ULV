#ifndef _ULVISOR_IOMEM_H_
#define _ULVISOR_IOMEM_H_

struct iomem_ops {
	int (*read)(void *data, int offset, void *res, int size);
	int (*write)(void *data, int offset, void *value, int size);
};

void *ulvisor_register_iomem(void *data, int size, const struct iomem_ops *ops);
void ulvisor_unregister_iomem(void *iomem_base);
void *ulvisor_ioremap(long addr, int size);
int ulvisor_iomem_access(const volatile void *addr, void *res, int size, int write);

#endif
