long lkl_syscall(int, long *);

static __inline__ long lkl_sys_umask(int mask) { long lkl_params[6] = { (long)mask }; return lkl_syscall(166, lkl_params); }
