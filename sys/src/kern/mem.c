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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/intr.h>
#include <sys/mem.h>
#include <sys/vm.h>

#define TEST_PATTERN    0xa5a5a5a5

/* Start and end of kernel ELF executable image loaded into memory */
extern char _start, _end;

size_t kernsize;
size_t memsize;
region_t regiontab;
region_t freelist, alloclist;

void
mem_init()
{
    u_long *ptr = (u_long *) 0x000ffffc;
    int regiontabsize, region_high, i;

    /* Determine kernel size */
    kernsize = ALIGN((u_long) & _end - (u_long) & _start, PAGE_SIZE);

    /* Determine physical memory size */
    do {
	ptr += 0x00100000;
	*ptr = (u_long) TEST_PATTERN;
    }
    while (*ptr == (u_long) TEST_PATTERN);
    memsize = (u_long) (ptr + 1) - 0x00100000;

    /* Allocate region table */
    regiontab = (region_t) REGION_LOW;
    regiontabsize = REGIONS * sizeof(struct region);

    regiontabsize = ALIGN(regiontabsize, PAGE_SIZE);

    /* Clear region table */
    for (i = 0; i < REGIONS; i++)
	region_clear(&(regiontab[i]));

    /* High region begins after the kernel code and data */
    region_high = ALIGN((u_long) & _end, PAGE_SIZE);

    /* Low memory is allocated for the initial region table */
    regiontab[0].start = REGION_LOW;
    regiontab[0].len = regiontabsize;
    regiontab[0].prev = NULL;
    regiontab[0].next = &(regiontab[2]);

    /* Region below video buffer is free */
    regiontab[1].start = REGION_LOW + regiontabsize;
    regiontab[1].len = REGION_VIDEO_START - (REGION_LOW + regiontabsize);
    regiontab[1].prev = NULL;
    regiontab[1].next = &(regiontab[3]);

    /* Video buffer and kernel regions are allocated */
    regiontab[2].start = REGION_VIDEO_START;
    regiontab[2].len = region_high - REGION_VIDEO_START;
    regiontab[2].prev = &(regiontab[0]);
    regiontab[2].next = NULL;

    /* Region above kernel is free */
    regiontab[3].start = region_high;
    regiontab[3].len = memsize - region_high;
    regiontab[3].prev = &(regiontab[1]);
    regiontab[3].next = NULL;

    /* Initial free and allocated lists */
    freelist = &(regiontab[1]);
    alloclist = &(regiontab[0]);
}

void *
mem_alloc(size_t size)
{
    region_t r;

    /* Increase size request to next multiple of page size */
    size = ALIGN(size, PAGE_SIZE);

    /* First-fit search for free region */
    for (r = freelist; r != NULL && r->len < size; r = r->next);
    if (r == NULL)
	return NULL;

    /* Split region into used and unused portions */
    if (r->len > size)
	region_split(r, size);

    /* Move used portion from free to allocated list */
    region_remove(r, &freelist);
    region_insert(r, &alloclist);

    /* Assign ownership of region to calling thread */
    if (current != NULL)
	r->proc = current->slot;

    return (void *) r->start;
}

int
mem_free(void *start)
{
    region_t r, s;

    if ((r = valid_region(start)) == NULL)
	return EINVAL;

    /* Move region from allocated to free list */
    region_remove(r, &alloclist);
    region_insert(r, &freelist);

    /* Clear ownership */
    r->proc = (-1);

    /* Merge with previous regions */
    for (s = r->prev;
	 s != NULL && s->start + s->len == r->start; r = s, s = s->prev) {
	s->len += r->len;
	region_remove(r, &freelist);
	region_clear(r);
    }

    /* Merge with next regions */
    for (s = r->next; s != NULL && r->start + r->len == s->start;
	 s = r->next) {
	r->len += s->len;
	region_remove(s, &freelist);
	region_clear(s);
    }
    return 0;
}

void
mem_reclaim(proc_t proc)
{
    region_t r;

    /* Free all allocated regions owned by specified process */
    for (r = alloclist; r != NULL;) {
	if (r->proc == proc->slot) {
	    mem_free((void *) r->start);
	    r = alloclist;
	    continue;
	}
	r = r->next;
    }
}

int
mem_transfer(void *start, proc_t proc)
{
    region_t r;

    if ((r = valid_region(start)) == NULL)
	return EINVAL;

    /* Transfer ownership of region to specified process */
    if (proc == NULL)
	r->proc = (-1);
    else
	r->proc = proc->slot;

    return 0;
}

void *
kmalloc(size_t size)
{
    void *ptr;
    struct vm_kmap_entry entry;

    disable;

    ptr = mem_alloc(size);
    if (ptr == NULL) {
	enable;
	return NULL;
    }
    mem_transfer(ptr, NULL);

    entry.start = ptr;
    entry.len = size;
    entry.attr = PTE_WRITE | PTE_PRESENT;
    vm_kmap_insert(&entry);
    enable;
    return ptr;
}

void
kfree(void *ptr)
{
    region_t r;
    struct vm_kmap_entry entry;

    disable;

    if ((r = valid_region(ptr)) == NULL) {
	enable;
	return;
    }
    entry.start = ptr;
    entry.len = r->len;
    entry.attr = 0;
    vm_kmap_remove(&entry);
    mem_free(ptr);
    enable;
}

void *
malloc(size_t size)
{
    void *ptr;

    disable;

    ptr = mem_alloc(size);
    if (ptr == NULL) {
	enable;
	return NULL;
    }
    vm_map_range((pt_t) current->context.tss->cr3, ptr, size,
		 PTE_USER | PTE_WRITE | PTE_PRESENT);
    enable;
    return ptr;
}

void
free(void *ptr)
{
    region_t r;

    disable;

    if ((r = valid_region(ptr)) == NULL) {
	enable;
	return;
    }
    vm_unmap_range((pt_t) current->context.tss->cr3, ptr, r->len);
    mem_free(ptr);
    enable;
}
