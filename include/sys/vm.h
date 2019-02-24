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

#ifndef __VM_H
#define __VM_H

#if _KERNEL

#include <sys/types.h>

#define PTE_PRESENT	0x01
#define PTE_WRITE	0x02
#define PTE_USER	0x04

#define IOMAP_ENTRIES	8

typedef u_long pte_t;
typedef pte_t *pt_t;

typedef struct pt_rec {
    struct pt_rec *next;
    int refcnt;
    pt_t pd;
} *pt_rec_t;

typedef struct vm_kmap_entry {
    void *start;
    u_long len;
    u_long attr;
} *vm_kmap_entry_t;

extern int npagetables;

void pt_push(pt_rec_t ptrec);
pt_rec_t pt_pop();
void vm_enable(u_long cr3);
u_long vm_pgfault_addr();
void dumpvm(pt_t pd);
void vm_init();
void vm_kmap(pt_t pd);
void vm_kmap_init();
int vm_kmap_insert(vm_kmap_entry_t entry);
int vm_kmap_remove(vm_kmap_entry_t entry);
int vm_map(pt_t pd, void *page, u_long attr);
void vm_map_init(pt_t pd);
int vm_map_range(pt_t pd, void *start, size_t len, u_long attr);
int vm_unmap(pt_t pd, void *page);
int vm_unmap_range(pt_t pd, void *start, size_t len);

#endif				/* _KERNEL */

#endif
