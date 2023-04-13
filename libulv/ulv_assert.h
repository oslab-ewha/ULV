#ifndef _ULV_ASSERT_H_
#define _ULV_ASSERT_H_

#define ULV_ASSERT(expr)	do { if (!(expr)) ulv_crash(); } while (0)
#define ULV_ABORT()	do { ulv_crash(); } while (0)
#define ULV_PANIC(fmt, ...)	do { ULV_VERBOSE(fmt, ## __VA_ARGS__); ulv_crash(); } while (0)
#define ULV_VERBOSE(fmt, ...)	do { ulv_verbose(fmt "\n", ## __VA_ARGS__); } while (0)

void ulv_crash(void);
void ulv_verbose(const char *fmt, ...);

#endif
