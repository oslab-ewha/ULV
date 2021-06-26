/* This file is part of GDBM, the GNU data base manager.
   Copyright 2016-2020 Free Software Foundation, Inc.

   GDBM is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GDBM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GDBM. If not, see <http://www.gnu.org/licenses/>.   */

#include "autoconf.h"
#include "gdbmdefs.h"
#include <ctype.h>

gdbm_debug_printer_t gdbm_debug_printer;
int gdbm_debug_flags;

struct gdbm_debug_token_desc
{
  char const *name;
  int flag;
};

struct gdbm_debug_token_desc const gdbm_debug_token_tab[] = {
  { "err",    GDBM_DEBUG_ERR },
  { "open",   GDBM_DEBUG_OPEN },
  { "store",  GDBM_DEBUG_STORE },
  { "read",   GDBM_DEBUG_READ },
  { "lookup", GDBM_DEBUG_LOOKUP },
  { "all",    GDBM_DEBUG_ALL },
  { NULL, 0 }
};

int
gdbm_debug_token (char const *tok)
{
  int i;

  for (i = 0; gdbm_debug_token_tab[i].name; i++)
    if (strcmp (gdbm_debug_token_tab[i].name, tok) == 0)
      return gdbm_debug_token_tab[i].flag;

  return 0;
}

void
gdbm_debug_parse_state (int (*f) (void *, int, char const *), void *d)
{
  int i;
  
  for (i = 0; gdbm_debug_token_tab[i].name; i++)
    {
      if (gdbm_debug_token_tab[i].flag == GDBM_DEBUG_ALL)
	continue;
      if (gdbm_debug_flags & gdbm_debug_token_tab[i].flag)
	{
	  if (f (d, gdbm_debug_token_tab[i].flag, gdbm_debug_token_tab[i].name))
	    break;
	}
    }
}

#define DATBUFSIZE 69

static int
datbuf_format (char vbuf[DATBUFSIZE], const char *buf, size_t size)
{
  char *p = vbuf;
  char *q = vbuf + 51;
  int i;
  size_t j = 0;
  static char hexchar[] = "0123456789ABCDEF";
  
  for (i = 0; i < 16; i++)
    {
      unsigned c;
      if (j < size)
	{
	  c = *(const unsigned char*)buf++;
	  j++;

	  *p++ = hexchar[c >> 4];
	  *p++ = hexchar[c & 0xf];
	  *p++ = ' ';
	  
	  *q++ = isprint (c) ? c : '.';
	  if (i == 7)
	    {
	      *p++ = ' ';
	      *q++ = ' ';
	    }
	}
      else
	{
	  *p++ = ' ';
	  *p++ = ' ';
	  *p++ = ' ';
	  *q++ = ' ';
	}
    }
  *p++ = ' ';
  *p = ' ';
  *q = 0;
  return j;
}

void
gdbm_debug_datum (datum dat, char const *pfx)
{
  char const *buf = dat.dptr;
  size_t size = dat.dsize;
  unsigned off;
  char vbuf[DATBUFSIZE];

  if (!buf)
    {
      gdbm_debug_printer ("%s%s\n", pfx, "NULL");
      return;
    }

  gdbm_debug_printer ("size=%d\n", size);
  off = 0;
  while (size)
    {
      size_t rd = datbuf_format (vbuf, buf, size);
      gdbm_debug_printer ("%s%04x:  %s\n", pfx, off, vbuf);
      size -= rd;
      buf += rd;
      off += rd;
    }
}

