/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

/*
 * This file provides an generic formatted printing function that is
 * used both in the kernel and the system library, libsys.a.  The file
 * is included in only two places, roadrunner/libsrc/libsys/print.c
 * and roadrunner/sys/src/kern/print.c, which is why the print()
 * function does not use the static definition.
 */

#include <ctype.h>
#include <dev/cons.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
put1 (char *s, int *pos, int c)
{
  if (s != NULL)
    {
      s[(*pos)++] = (char) c;
      return;
    }
#if _KERNEL
  put ((char) c);
#else
  write (STDOUT, &c, 1);
#endif
}

void
print (char *string, int *pos, const char *fmt, va_list args)
{
  int len;
  char pad;

printloop:
  if (*fmt == '\0')
    goto printend;

  if (*fmt != '%')
    {
      put1 (string, pos, (int) *(fmt++));
      goto printloop;
    }
  fmt++;

  if (*fmt == '0')
    {
      pad = '0';
      while (*(++fmt) == '0');
    }
  else
    pad = ' ';

  len = 0;
  while (isdigit (*fmt))
    len = len * 10 + (*(fmt++) - '0');

  if (*fmt == 'h' || *fmt == 'l' || *fmt == 'q' || *fmt == 'L')
    fmt++;

  switch (*fmt)
    {
    case 'd':
      {
	char s[80];
	int v, l, i;

	v = va_arg (args, int);

	uint2str ((v < 0 ? (u_int) - v : (u_int) v), s, 10);

	if (len != 0 && (l = strlen (s)) <= len)
	  {
	    int padlen;

	    if (v < 0)
	      padlen = len - l - 1;
	    else
	      padlen = len - l;

	    for (i = 0; i < padlen; i++)
	      put1 (string, pos, (int) pad);
	  }
	if (v < 0)
	  put1 (string, pos, (int) '-');
	for (i = 0; s[i] != '\0'; i++)
	  put1 (string, pos, (int) s[i]);
	fmt++;
      }
      break;

    case 'u':
    case 'x':
      {
	char s[80];
	u_int v;
	int l, i;

	v = va_arg (args, u_int);
	uint2str (v, s, (*fmt == 'x' ? 16 : 10));

	if (len != 0 && (l = strlen (s)) <= len)
	  for (i = 0; i < len - l; i++)
	    put1 (string, pos, (int) pad);

	for (i = 0; s[i] != '\0'; i++)
	  put1 (string, pos, (int) s[i]);

	fmt++;
      }
      break;

    case 's':
      {
	char *s;
	int l, i;

	s = va_arg (args, char *);

	i = 0;
	if (len == 0)
	  {
	    while (s[i] != '\0')
	      put1 (string, pos, (int) s[i++]);
	  }
	else
	  {
	    while (i < len && s[i] != '\0')
	      put1 (string, pos, (int) s[i++]);
	  }

	if (len != 0 && (l = strlen (s)) <= len)
	  for (i = 0; i < len - l; i++)
	    put1 (string, pos, (int) ' ');

	fmt++;
      }
      break;

    case 'c':
      {
	int c = va_arg (args, int);

	put1 (string, pos, c);
	fmt++;
      }
      break;
    case '%':
      put1 (string, pos, (int) '%');
      fmt++;
      break;
    default:
      put1 (string, pos, (int) *(fmt++));
    }

  goto printloop;

printend:
  if (string != NULL)
    put1 (string, pos, (int) '\0');
}
