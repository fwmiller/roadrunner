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

#ifndef __MEM_H
#define __MEM_H

#if _KERNEL

#include <sys/proc.h>
#include <sys/types.h>

/* 
 * XXX This is really ugly, but it works (tm).  The 4 Kbyte page located at
 * 0x1000 is reserved.  It is used to pass data between the boot program and
 * as a bounce buffer for the floppy disk driver.
 */
#define REGION_LOW		0x00002000
#define REGION_VIDEO_START	0x000a0000
#define REGION_VIDEO_END	0x00100000

struct region {
    struct region *prev, *next;
    u_long start;		       /* Starting address of region */
    size_t len;			       /* Length of region */
    int proc;			       /* Thread that owns the region */
};

typedef struct region *region_t;

extern size_t kernsize, memsize;
extern region_t regiontab, freelist, alloclist;

void dumpregiontab();
region_t valid_region(void *start);
void region_clear(region_t r);
void region_insert(region_t r, region_t * l);
void region_remove(region_t r, region_t * l);
void *region_split(region_t r, size_t size);

void *kmalloc(size_t size);
void kfree(void *ptr);
void *malloc(size_t size);
void free(void *ptr);

void *mem_alloc(size_t size);
int mem_free(void *start);
void mem_init();
void mem_reclaim(proc_t proc);
int mem_transfer(void *start, proc_t proc);

#endif				/* _KERNEL */

#endif
