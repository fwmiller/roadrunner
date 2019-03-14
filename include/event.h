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

#ifndef __EVENT_H
#define __EVENT_H

#include <sys/proc.h>

/* Events tied to hardware interrupts */
#define EVENT_TMR		0
#define EVENT_KBD		1
#define EVENT_SERALT		2
#define EVENT_SERPRI		3
#define EVENT_FD		4
#define EVENT_PARA		5
#define EVENT_RTC		6
#define EVENT_AUX		7
#define EVENT_MATHCOPRERR	8
#define EVENT_HD		9

/* Kernel-specific events */
#define EVENT_BUF		10
#define EVENT_IP		11

extern struct queue eventtab[];

#if _KERNEL
void eventtab_init ();
#endif
int event_inst (int event, proc_t proc);
int event_raise (int event);
int event_uninst (int event, proc_t proc);
int event_wait (int event);

#endif
