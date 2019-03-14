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

#ifndef __SEGMENT_H
#define __SEGMENT_H

#if _KERNEL

#include <sys/types.h>

struct pseudodesc
{
  u_long eip;
  u_short cs;
} __attribute__ ((packed));

struct tss
{
  u_long back;
  u_long esp0;
  u_long ss0;
  u_long esp1;
  u_long ss1;
  u_long esp2;
  u_long ss2;
  u_long cr3;
  u_long eip;
  u_long eflags;
  u_long eax;
  u_long ecx;
  u_long edx;
  u_long ebx;
  u_long esp;
  u_long ebp;
  u_long esi;
  u_long edi;
  u_long es;
  u_long cs;
  u_long ss;
  u_long ds;
  u_long fs;
  u_long gs;
  u_long ldt;
  u_long ioperm;
} __attribute__ ((packed));

struct tssdesc
{
  u_short limit0;
  u_short base0;
  u_char base16;
  u_char type;
  u_char limit16;
  u_char base24;
} __attribute__ ((packed));

typedef struct tss *tss_t;
typedef struct tssdesc *tssdesc_t;

extern u_char gdt[];
extern u_char gdtptr[];
extern struct tss tsstab[];
extern tssdesc_t tssdesctab;

void user_data_segs ();
void kern_data_segs ();
void gdtinit ();

void ktssinit (tss_t tss, u_long kstk, size_t kstksize);

void
tssinit (tss_t tss, u_long kstk, size_t kstksize, u_long stk, size_t stksize);

void dumptss (tss_t tss);

void ltr (u_long selector);
void jmptss (void *tssdesc);

#endif /* _KERNEL */

#endif
