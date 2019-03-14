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

#include <ctype.h>
#include <dev/cons.h>
#include <string.h>

#define LINES		25
#define COLS		80

#define BASE		((char *) 0xb8000)
#define END		((char *) BASE + LINES * 2 * COLS)

#define CURSOR		' '
#define NORMAL		0x07
#define REVERSE		0x70

static char *cursor;

void clear()
{
	for (cursor = BASE; cursor < END; cursor += 2) {
		*cursor = ' ';
		*(cursor + 1) = NORMAL;
	}
	cursor = BASE;

	*cursor = CURSOR;
	*(cursor + 1) = REVERSE;
}

static void scroll()
{
	char *i, *j;

	for (i = BASE, j = BASE + 2 * COLS; j < END;
	     i += 2 * COLS, j += 2 * COLS)
		bcopy(j, i, 2 * COLS);

	while (i < END) {
		*(i + 1) = NORMAL;
		*i = ' ';
		i += 2;
	}
	cursor = END - 2 * COLS;

	*cursor = CURSOR;
	*(cursor + 1) = REVERSE;
}

void put(char c)
{
	if (isprint(c)) {
		*(cursor + 1) = NORMAL;
		*cursor = c;
		cursor += 2;
		if (cursor >= END)
			scroll();

	} else if (c == '\n') {
		*(cursor + 1) = NORMAL;
		*cursor = ' ';
		if (cursor < END - 2 * COLS) {
			char *i = BASE;

			while (i <= cursor)
				i += 2 * COLS;
			cursor = i;
		} else
			scroll();
	}
	*cursor = CURSOR;
	*(cursor + 1) = REVERSE;
}
