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

#include <string.h>
#include "vi.h"

/*
 * The text buffer is organized as a simple array of characters that grows
 * on demand.  The array is a sequence of lines where each line is
 * terminated by a '\n' character.  If a character is inserted all the
 * characters to the right of it shift right by one position.  When a
 * character is deleted, all the characters to the right of it shift to
 * the left one position.
 */

int
dist_to_start_of_line(int pos)
{
    int dist;

    for (dist = 0; pos > 0 && buf[--pos] != '\n'; dist++);
    return dist;
}

int
dist_to_end_of_line(int pos)
{
    int dist;

    for (dist = 0; pos < buflen && buf[pos++] != '\n'; dist++);
    return dist;
}

void
shift_left(char *dst, char *src, int len)
{
    if (len <= 0)
	return;

    bcopy(src, dst, len);
    src[len - 1] = 0;
}

void
shift_right(char *dst, char *src, int len)
{
    int i;

    if (len <= 0)
	return;

    for (i = len - 1; i >= 0; i--)
	dst[i] = src[i];
    src[0] = 0;
}
