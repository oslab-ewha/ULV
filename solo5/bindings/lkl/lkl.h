long lkl_syscall(long, long *);

struct utsname {
	char sysname[9];
	char nodename[24];
	char release[32];
	char version[64];
	char machine[64];
	char dummy[1024];
};

static __inline__ long lkl_sys_uname(struct utsname *name) { long lkl_params[6] = { (long)name }; return lkl_syscall(160, lkl_params); }
static __inline__ long lkl_sys_umask(int mask) { long lkl_params[6] = { (long)mask }; return lkl_syscall(166, lkl_params); }
