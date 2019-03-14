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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mem.h>

#define REGION_HDR "addr       pages  pid"

static void dump_region(char **s, region_t region, int free)
{
	sprintf(*s, "%08x", (u_int) region->start);
	*s += 8;

	if (free)
		sprintf(*s, "*  ");
	else
		sprintf(*s, "   ");
	*s += 3;

	sprintf(*s, "%5u  ", (u_int) region->len / PAGE_SIZE);
	*s += 7;

	if (region->proc < 0)
		sprintf(*s, "     -");
	else
		sprintf(*s, "%6d", region->proc);
	*s += 6;

	sprintf(*s, "\n");
	(*s)++;
}

void dump_regiontab(char *s)
{
	region_t r1, r2;
	int regioncnt = 0;
	char s0[80];

	for (r1 = freelist, r2 = alloclist;;) {
		if (r1 == NULL && r2 == NULL)
			break;
		if (r1 == NULL)
			goto r2print;
		if (r2 == NULL || r1->start < r2->start)
			goto r1print;
 r2print:
		if (regioncnt++ == 0) {
			sprintf(s0, "%s\n", REGION_HDR);
			strcat(s, s0);
			s += strlen(s0);
		}
		dump_region(&s, r2, 0);
		r2 = r2->next;
		continue;
 r1print:
		if (regioncnt++ == 0) {
			sprintf(s0, "%s\n", REGION_HDR);
			strcat(s, s0);
			s += strlen(s0);
		}
		dump_region(&s, r1, 1);
		r1 = r1->next;
	}
}
