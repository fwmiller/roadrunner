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

#ifndef __SYS_H
#define __SYS_H

#include <sys/mc146818.h>
#include <sys/types.h>

#define ALIGN(V, SIZE) ((((V) + (SIZE) - 1) / (SIZE)) * (SIZE))

#if _KERNEL

#define inb(port)							\
({									\
    register int _inb_result;						\
    asm volatile ("xorl %%eax,%%eax; inb %%dx,%%al" :			\
		  "=a" (_inb_result) : "d" (port));			\
    _inb_result;							\
})

#define inw(port)							\
({									\
    register int _inb_result;						\
    asm volatile ("xorl %%eax,%%eax; inw %%dx,%%ax" :			\
		  "=a" (_inb_result) : "d" (port));			\
    _inb_result;							\
})

#define inl(port)							\
({									\
    register unsigned long _inb_result;					\
    asm volatile ("xorl %%eax,%%eax; inl %%dx,%%eax" :			\
		  "=a" (_inb_result) : "d" (port));			\
    _inb_result;							\
})

#define outb(port, data)						\
    asm volatile ("outb %%al,%%dx" : : "a" (data), "d" (port))

#define outw(port, data)						\
    asm volatile ("outw %%ax,%%dx" : : "a" (data), "d" (port))

#define outl(port, data)						\
    asm volatile ("outl %%eax,%%dx" : : "a" (data), "d" (port))

static inline void
insb (u_short port, u_char * dst, int bytes)
{
  int i;

  for (i = 0; i < bytes; i++)
    dst[i] = inb (port);
}

static inline void
insw (u_short port, u_short * dst, int words)
{
  int i;

  for (i = 0; i < words; i++)
    ((u_short *) dst)[i] = inw (port);
}

static inline void
insl (u_short port, u_long * dst, int dwords)
{
  int i;

  for (i = 0; i < dwords; i++)
    ((u_int *) dst)[i] = inl (port);
}

static inline void
outsb (u_short port, u_char * src, int bytes)
{
  int i;

  for (i = 0; i < bytes; i++)
    outb (port, src[i]);
}

static inline void
outsw (u_short port, u_short * src, int words)
{
  int i;

  for (i = 0; i < words; i++)
    outw (port, ((u_short *) src)[i]);
}

static inline void
outsl (u_short port, u_long * src, int dwords)
{
  int i;

  for (i = 0; i < dwords; i++)
    outl (port, ((u_int *) src)[i]);
}

int bcd2int (u_char v);

static inline u_char
loadcmosbyte (u_char field)
{
  outb (MC146818_ADDR, field);
  return inb (MC146818_DATA);
}

#endif /* _KERNEL */

u_char loadbyte (u_char * ptr);
u_int loadword (u_char * ptr);
u_int loaddword (u_char * ptr);

void storebyte (u_char val, u_char * ptr);
void storeword (u_int val, u_char * ptr);
void storedword (u_int val, u_char * ptr);

void halt ();
void reboot ();
int load (char *path, char **prog, u_long * size, char **start);

#endif
