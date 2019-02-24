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

#define DUMP_REGION							\
{									\
    if (regioncnt++ == 0) {						\
	sprintf(s, "addr      attr  pages\n");				\
	s += 22;							\
    }									\
    sprintf(s, "%08x  ", (u_int) start);				\
    s += 10;								\
    if (attr & PTE_WRITE)						\
	sprintf(s, "w");						\
    else								\
	sprintf(s, "r");						\
    s++;								\
    if (attr & PTE_USER)						\
	sprintf(s, "u");						\
    else								\
	sprintf(s, "s");						\
    s++;								\
    sprintf(s0, "    %u\n",						\
	    ((u_int) page - (u_int) start) / PAGE_SIZE);		\
    strcat(s, s0);							\
    s += strlen(s0);							\
}

void
dump_vm(char *s, pt_t pd)
{
    u_long page, start = 0, attr;
    pt_t pt;
    pte_t pte;
    int pti, regioncnt = 0;
    char s0[80];

    for (page = 0; page < memsize; page += PAGE_SIZE) {
	pti = (int) ((u_long) page >> 22) & 0x03ff;
	pt = (pt_t) (((pte_t) pd[pti]) & 0xfffff000);
	if (pt == NULL)
	    continue;
	pti = (int) ((u_long) page >> 12) & 0x03ff;
	pte = (pte_t) pt[pti];
	if (start == 0 && (pte & PTE_PRESENT)) {
	    start = page;
	    attr = pte & (PTE_WRITE | PTE_USER);

	} else if (start > 0 &&
		   (!(pte & PTE_PRESENT) ||
		    ((pte & (PTE_WRITE | PTE_USER)) != attr))) {
	    DUMP_REGION;

	    if (!(pte & PTE_PRESENT))
		start = 0;
	    else {
		start = page;
		attr = pte & (PTE_WRITE | PTE_USER);
	    }
	}
    }
    if (start > 0)
	DUMP_REGION;
}
