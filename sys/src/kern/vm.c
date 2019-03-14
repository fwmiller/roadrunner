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
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/mem.h>
#include <sys/config.h>
#include <sys/vm.h>

/* Start of kernel ELF executable image loaded into memory */
extern char _start;

static pt_rec_t ptstk = NULL;
static char *pagetablerecords;
static char *pagetables;
static struct vm_kmap_entry kmaptab[VM_KMAP_ENTRIES];
static int nextkmaptabentry = 0;

int npagetables;

void pt_push(pt_rec_t ptrec)
{
	ptrec->next = ptstk;
	ptstk = ptrec;
}

pt_rec_t pt_pop()
{
	pt_rec_t ptrec = ptstk;

	if (ptrec != NULL)
		ptstk = ptrec->next;
	return ptrec;
}

void vm_init()
{
	pt_rec_t ptrec;
	struct vm_kmap_entry entry;
	int npages, i;

	pagetablerecords = mem_alloc(PAGE_TABLES * sizeof(struct pt_rec));
	bzero(pagetablerecords, PAGE_TABLES * sizeof(struct pt_rec));

	npages = (memsize + PAGE_SIZE - 1) / PAGE_SIZE;
	npagetables = (npages / 1024) + (npages % 1024 > 0 ? 1 : 0);

	pagetables = mem_alloc(PAGE_TABLES * (npagetables + 1) * PAGE_SIZE);
	bzero(pagetables, PAGE_TABLES * (npagetables + 1) * PAGE_SIZE);

	for (i = 0; i < PAGE_TABLES; i++) {
		ptrec =
		    (pt_rec_t) (pagetablerecords + i * sizeof(struct pt_rec));

		ptrec->pd =
		    (pt_t) (pagetables + i * (npagetables + 1) * PAGE_SIZE);
		pt_push(ptrec);
	}
	vm_kmap_init();

	entry.start = (void *)0x1000;
	entry.len = 18 * PAGE_SIZE;
	entry.attr = PTE_WRITE | PTE_PRESENT;
	vm_kmap_insert(&entry);

	entry.start = (void *)REGION_VIDEO_START;
	entry.len = REGION_VIDEO_END - REGION_VIDEO_START;
	entry.attr = PTE_WRITE | PTE_PRESENT;
	vm_kmap_insert(&entry);

	entry.start = (void *)&_start;
	entry.len = kernsize;

	/*
	 * XXX Temporary until I find a better way to do this.  The PTE_USER
	 * flag means that all the kernel code is readable by user processes.
	 * This is necessary because the first little stub of code that gets
	 * run by a new thread created using proc_exec() is a routine called
	 * proc_start(), which is in kernel memory.  I really need to figure
	 * a way around this problem.  The kernel code can't be damaged since
	 * its read-only, but we really don't want it to be read accessible
	 * either.
	 */
	entry.attr = PTE_USER | PTE_PRESENT;

	vm_kmap_insert(&entry);

	entry.start = regiontab;
	entry.len = REGIONS * sizeof(struct region);

	entry.attr = PTE_WRITE | PTE_PRESENT;
	vm_kmap_insert(&entry);

	entry.start = pagetablerecords;
	entry.len = PAGE_TABLES * sizeof(struct pt_rec);

	entry.attr = PTE_WRITE | PTE_PRESENT;
	vm_kmap_insert(&entry);

	entry.start = pagetables;
	entry.len = PAGE_TABLES * (npagetables + 1) * PAGE_SIZE;
	entry.attr = PTE_WRITE | PTE_PRESENT;
	vm_kmap_insert(&entry);
}

void vm_kmap(pt_t pd)
{
	vm_kmap_entry_t entry;
	int i;

	for (i = 0; i < VM_KMAP_ENTRIES; i++) {
		entry = &(kmaptab[i]);
		if (entry->start != NULL) {
			if ((u_long) entry->start < memsize)
				vm_map_range(pd, entry->start, entry->len,
					     entry->attr);
			else {
#if _DEBUG
				kprintf("vm_kmap: iomap entry %08x len %u\n",
					entry->start, entry->len);
#endif
			}
		}
	}
}

void vm_kmap_init()
{
	bzero(kmaptab, VM_KMAP_ENTRIES * sizeof(struct vm_kmap_entry));
}

int vm_kmap_insert(vm_kmap_entry_t entry)
{
	int firstkmaptabentry, found = 0, i;

	/* Check for duplicate entry */
	for (i = 0; i < VM_KMAP_ENTRIES; i++)
		if (kmaptab[i].start == entry->start)
			return EKMAPDUP;
	/* Search for a free kmap table entry */
	for (firstkmaptabentry = nextkmaptabentry;;) {
		if (kmaptab[nextkmaptabentry].start == NULL) {
			found = 1;
			break;
		}
		nextkmaptabentry = (nextkmaptabentry + 1) % VM_KMAP_ENTRIES;
		if (nextkmaptabentry == firstkmaptabentry)
			break;
	}
	if (!found)
		return EAGAIN;

	/* Fill in kmap table entry */
	kmaptab[nextkmaptabentry].start = entry->start;
	kmaptab[nextkmaptabentry].len = entry->len;
	kmaptab[nextkmaptabentry].attr = entry->attr;
	nextkmaptabentry = (nextkmaptabentry + 1) % VM_KMAP_ENTRIES;

	/* Update page tables of all existing processes */
	for (i = 0; i < PROCS; i++)
		if (proctab[i].state != PS_NULL) {
			if ((u_long) entry->start < memsize)
				vm_map_range((pt_t) proctab[i].context.tss->cr3,
					     entry->start, entry->len,
					     entry->attr);
			else {
#if _DEBUG
				kprintf
				    ("vm_kmap_insert: iomap entry %08x len %u\n",
				     entry->start, entry->len);
#endif
			}
		}
	return 0;
}

int vm_kmap_remove(vm_kmap_entry_t entry)
{
	int kmaptabentry, i;

	/* Search for corrensponding kmap table entry */
	for (kmaptabentry = 0; kmaptabentry < VM_KMAP_ENTRIES; kmaptabentry++)
		if (kmaptab[kmaptabentry].start == entry->start
		    && kmaptab[kmaptabentry].len == entry->len)
			break;
	if (kmaptabentry == VM_KMAP_ENTRIES)
		return EINVAL;

	/* Clear kmap table entry */
	bzero(&(kmaptab[kmaptabentry]), sizeof(struct vm_kmap_entry));

	/* Update page tables of all existing processes */
	for (i = 0; i < PROCS; i++)
		if (proctab[i].state != PS_NULL)
			vm_unmap_range((pt_t) proctab[i].context.tss->cr3,
				       kmaptab[kmaptabentry].start,
				       kmaptab[kmaptabentry].len);

	return 0;
}

void vm_map_init(pt_t pd)
{
	u_long addr;
	int i;

	bzero(pd, (npagetables + 1) * PAGE_SIZE);

	for (i = 0; i < npagetables; i++) {
		addr = (u_long) pd + ((i + 1) * PAGE_SIZE);
		pd[i] = ((pte_t) addr & 0xfffff000) |
		    PTE_USER | PTE_WRITE | PTE_PRESENT;
	}
}

__inline int vm_map(pt_t pd, void *page, u_long attr)
{
	pt_t pt;
	pte_t addr;
	int pti;

	if (pd == NULL || page == NULL)
		return EINVAL;

	pti = (int)((u_long) page >> 22) & 0x03ff;
	addr = (pte_t) pd[pti] & 0xfffff000;
	pt = (pt_t) addr;

	pti = (int)((u_long) page >> 12) & 0x03ff;
	addr = (pte_t) pt[pti] & 0xfffff000;

	if (addr == (pte_t) page)
		return 0;

	if (addr != (pte_t) NULL)
		return EPTDUP;

	pt[pti] = ((pte_t) page & 0xfffff000) | attr;

	return 0;
}

int vm_map_range(pt_t pd, void *start, size_t len, u_long attr)
{
	u_long addr, end;
	int result;

	len = ALIGN(len, PAGE_SIZE);
	addr = (u_long) start;
	end = addr + len;

	for (; addr < end; addr += PAGE_SIZE)
		if ((result = vm_map(pd, (void *)addr, attr)) < 0)
			return result;
	return 0;
}

__inline int vm_unmap(pt_t pd, void *page)
{
	pt_t pt;
	pte_t addr;
	int pti;

	if (pd == NULL || page == NULL)
		return EINVAL;

	pti = (int)((u_long) page >> 22) & 0x03ff;
	addr = (pte_t) pd[pti] & 0xfffff000;
	pt = (pt_t) addr;

	pti = (int)((u_long) page >> 12) & 0x03ff;
	addr = (pte_t) pt[pti] & 0xfffff000;

	if (addr != (pte_t) page)
		return EPTCORRUPT;

	pt[pti] = 0;

	return 0;
}

int vm_unmap_range(pt_t pd, void *start, size_t len)
{
	u_long addr, end;
	int result;

	len = ALIGN(len, PAGE_SIZE);
	addr = (u_long) start;
	end = addr + len;

	for (; addr < end; addr += PAGE_SIZE)
		if ((result = vm_unmap(pd, (void *)addr)) < 0)
			return result;
	return 0;
}
