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

#ifndef __TIMER_H
#define __TIMER_H

#if _KERNEL

#define TIMER_ISR_PERIOD	100000 /* usec */

typedef void (*timer_func_t) (void *);

void timer_isr();
void timer_init();
int timer_start(char *name, timeval_t dur, timer_func_t f, void *arg);
int timer_loop(char *name, timeval_t period, timer_func_t f, void *arg);
int timer_cancel(char *name);

#endif				/* _KERNEL */

#endif
