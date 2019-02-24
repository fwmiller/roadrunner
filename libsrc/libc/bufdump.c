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
#include <stdio.h>
#include <sys/types.h>

#define LEN	8

void
bufdump(char *buf, int size)
{
    u_char *line;
    int i, j, lines;

    lines = (size + LEN - 1) / LEN;
    for (i = 0; i < lines; i++) {
	line = (u_char *) buf + i * LEN;
	printf("%08x  ", (u_int) buf + i * LEN);
	for (j = 0; j < LEN; j++)
	    printf("%02x ", line[j]);
	printf(" ");
	for (j = 0; j < LEN; j++)
	    if (isprint(line[j]))
		printf("%c", (char) line[j]);
	    else
		printf(".");
	printf("\n");
    }
}
