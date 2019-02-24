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
#include <string.h>
#include <sys/config.h>
#include <sys/segment.h>
#include <sys/proc.h>
#include <sys/selector.h>

u_char gdt[(GDT_SIZE + PROCS) * SEL_SIZE];
u_char gdtptr[6];
struct tss tsstab[PROCS];
tssdesc_t tssdesctab;

void
gdtinit()
{
    tss_t tss;
    tssdesc_t desc;
    int i;

    bzero(gdt, (GDT_SIZE + PROCS) * SEL_SIZE);
    bzero(gdtptr, 6);
    bzero(tsstab, PROCS * sizeof(struct tss));

    /* Kernel code descriptor */
    *((u_short *) (gdt + KCSEL)) = 0xffff;
    *(gdt + KCSEL + 5) = 0x9b;
    *(gdt + KCSEL + 6) = 0xcf;

    /* Kernel data descriptor */
    *((u_short *) (gdt + KDSEL)) = 0xffff;
    *(gdt + KDSEL + 5) = 0x93;
    *(gdt + KDSEL + 6) = 0xcf;

    /* User code descriptor */
    *((u_short *) (gdt + UCSEL)) = 0xffff;
    *(gdt + UCSEL + 5) = 0xfb;
    *(gdt + UCSEL + 6) = 0xcf;

    /* User data descriptor */
    *((u_short *) (gdt + UDSEL)) = 0xffff;
    *(gdt + UDSEL + 5) = 0xf3;
    *(gdt + UDSEL + 6) = 0xcf;

    /* Setup GDT pointer */
    *((u_short *) gdtptr) = ((GDT_SIZE + PROCS) * SEL_SIZE) - 1;
    *((u_long *) (gdtptr + 2)) = (u_long) gdt;

    /* Location of TSS descriptors in GDT */
    tssdesctab = (tssdesc_t) (gdt + (GDT_SIZE * SEL_SIZE));

    /* Setup TSS descriptors */
    for (i = 0; i < PROCS; i++) {
	tss = &(tsstab[i]);
	desc = &(tssdesctab[i]);
	desc->limit0 = 103;
	desc->base0 = (u_short) ((u_long) tss & 0xffff);
	desc->base16 = (u_char) ((((u_long) tss) >> 16) & 0xff);
	desc->type = 0x89;
	desc->limit16 = 0x80;
	desc->base24 = (u_char) ((((u_long) tss) >> 24) & 0xff);
    }
}

void
ktssinit(tss_t tss, u_long kstk, size_t kstksize)
{
    tss->back = 0;
    tss->esp0 = 0;
    tss->ss0 = 0;
    tss->esp1 = 0;
    tss->ss1 = 0;
    tss->esp2 = 0;
    tss->ss2 = 0;
    tss->eip = (u_long) proc_start;
    tss->eflags = 0x200;	       /* interrupts enabled */
    tss->eax = 0;
    tss->ecx = 0;
    tss->edx = 0;
    tss->ebx = 0;
    tss->esp = kstk + kstksize;
    tss->ebp = kstk + kstksize;
    tss->esi = 0;
    tss->edi = 0;

    tss->es = KDSEL;
    tss->cs = KCSEL;
    tss->ss = KDSEL;
    tss->ds = KDSEL;
    tss->fs = KDSEL;
    tss->gs = KDSEL;

    tss->ldt = 0;
    tss->ioperm = 0;
}

void
tssinit(tss_t tss, u_long kstk, size_t kstksize, u_long stk, size_t stksize)
{
    tss->back = 0;
    tss->esp0 = kstk + kstksize;
    tss->ss0 = KDSEL;
    tss->esp1 = 0;
    tss->ss1 = 0;
    tss->esp2 = 0;
    tss->ss2 = 0;
    tss->eip = (u_long) proc_start;
    tss->eflags = 0x200;	       /* interrupts enabled */
    tss->eax = 0;
    tss->ecx = 0;
    tss->edx = 0;
    tss->ebx = 0;
    tss->esp = stk + stksize;
    tss->ebp = stk + stksize;
    tss->esi = 0;
    tss->edi = 0;

    tss->es = UDSEL | 0x03;
    tss->cs = UCSEL | 0x03;
    tss->ss = UDSEL | 0x03;
    tss->ds = UDSEL | 0x03;
    tss->fs = UDSEL | 0x03;
    tss->gs = UDSEL | 0x03;

    tss->ldt = 0;
    tss->ioperm = 0;
}

#if _DEBUG
void
dumptss(tss_t tss)
{
    kprintf("ss0 %04x  esp0 %08x\n", (u_int) tss->ss0, (u_int) tss->esp0);
    kprintf("cr3 %08x  eip %08x  eflags %08x\n", (u_int) tss->cr3,
	    (u_int) tss->eip, (u_int) tss->eflags);
    kprintf("eax %08x  ebx %08x  ecx %08x  edx %08x\n",
	    (u_int) tss->eax, (u_int) tss->ebx, (u_int) tss->ecx,
	    (u_int) tss->edx);
    kprintf("esi %08x  edi %08x  esp %08x  ebp %08x\n",
	    (u_int) tss->esi, (u_int) tss->edi, (u_int) tss->esp,
	    (u_int) tss->ebp);
    kprintf("cs %04x  ss %04x  ds %04x  es %04x  fs %04x  gs %04x\n",
	    (u_int) tss->cs, (u_int) tss->ss, (u_int) tss->ds,
	    (u_int) tss->es, (u_int) tss->fs, (u_int) tss->gs);
}
#endif
