#ifndef _LIBLKL_H_
#define _LIBLKL_H_

extern void init_liblkl(void);
extern char *mount_fs(const char *path_host);

long lkl_syscall(long, long *);

struct utsname {
	char sysname[9];
	char nodename[24];
	char release[32];
	char version[64];
	char machine[64];
	char dummy[1024];
};

struct timeval {
	unsigned long long	tv_sec;
	unsigned long long	tv_uec;
};

static __inline__ long lkl_sys_open(const char *path, int flags) { long lkl_params[6] = { (long)0, (long)path, (long)flags }; return lkl_syscall(56, lkl_params); }
static __inline__ long lkl_sys_close(int fd) { long lkl_params[6] = { (long)fd }; return lkl_syscall(57, lkl_params); }
static __inline__ long lkl_sys_uname(struct utsname *name) { long lkl_params[6] = { (long)name }; return lkl_syscall(160, lkl_params); }
static __inline__ long lkl_sys_umask(int mask) { long lkl_params[6] = { (long)mask }; return lkl_syscall(166, lkl_params); }
static __inline__ long lkl_sys_gettimeofday(struct timeval *tv) { long lkl_params[6] = { (long)tv, (long)0 }; return lkl_syscall(169, lkl_params); }

int strlen(const char *str);
int snprintf(char *str, size_t size, const char *format, ...);

#endif
