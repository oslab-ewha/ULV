#include "ulfs_p.h"

inline void
ulfs_path_init(path_t *ppath, const char *path)
{
	const char	*p;

	ppath->start = path;
	for (p = path; *p; p++);
	ppath->end = p;
	ppath->start_name = NULL;
}

inline void
ulfs_path_dirname(path_t *ppath)
{
	const char	*p;

	for (p = ppath->end - 1; p >= ppath->start && *p != '/'; p--);
	if (p >= ppath->start && *p == '/') {
		if (p == ppath->start)
			ppath->end = p + 1;
		else
			ppath->end = p;
	}
	else
		ppath->end = ppath->start;
}

inline void
ulfs_path_basename(path_t *ppath)
{
	const char	*p;

	for (p = ppath->end - 1; p >= ppath->start && *p != '/'; p--);
	ppath->start = p + 1;
}

inline void
ulfs_path_first_name(path_t *ppath)
{
	const char	*p;

	for (p = ppath->start; *p && *p != '/'; p++);
	ppath->start_name = ppath->start;
	ppath->end_name = p;
}

inline bool_t
ulfs_path_next_name(path_t *ppath)
{
	const char	*p;

	if (ppath->end_name == ppath->end) {
		ppath->start_name = NULL;
		return FALSE;
	}

	for (p = ppath->end_name + 1; *p && *p != '/'; p++);
	ppath->start_name = ppath->end_name + 1;
	ppath->end_name = p;

	return TRUE;
}

inline bool_t
ulfs_path_matched(path_t *ppath, const char *name)
{
	const char	*p, *q;
	const char	*start = ppath->start_name, *end = ppath->end_name;

	if (start == NULL) {
		start = ppath->start;
		end = ppath->end;
	}

	for (p = start, q = name; p < end; p++, q++) {
		if (*p != *q)
			return FALSE;
	}
	if (*q != '\0')
		return FALSE;
	return TRUE;
}

inline bool_t
ulfs_path_is_empty(path_t *ppath)
{
	if (ppath->start == ppath->end)
		return TRUE;
	return FALSE;
}

inline bool_t
ulfs_path_is_root(path_t *ppath)
{
	if (ppath->start + 1 != ppath->end)
		return FALSE;
	if (*ppath->start != '/')
		return FALSE;
	return TRUE;
}

inline bool_t
ulfs_path_is_abs(path_t *ppath)
{
	if (ppath->start == ppath->end)
		return FALSE;
	if (*ppath->start != '/')
		return FALSE;
	return TRUE;
}
