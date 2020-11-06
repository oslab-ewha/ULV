extern void lkl_start_kernel_default(void);
extern void __init_libc(char *envp[], const char *);

void
init_lkl(void)
{
	char	*envp[] = { 0 };

	__init_libc(envp, "lkl");

	lkl_start_kernel_default();
}
