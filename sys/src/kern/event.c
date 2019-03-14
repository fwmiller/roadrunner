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
#include <sys/config.h>
#include <sys/intr.h>
#include <sys/queue.h>

struct queue eventtab[EVENTS];

void eventtab_init()
{
	int i;

	for (i = 0; i < EVENTS; i++)
		initq(&(eventtab[i]));
}

int event_inst(int event, proc_t proc)
{
	if (event < 0 || event >= EVENTS)
		return EINVAL;

	disable;

	if (proc == NULL)
		proc = current;

	proc->state = PS_EVENT;
	insq(proc, &(eventtab[event]));

	if (proc == current)
		proc_transfer();	/* enables interrupts */
	else
		enable;

	return 0;
}

int event_raise(int event)
{
	proc_t proc;

	if (event < 0 || event >= EVENTS)
		return EINVAL;

	disable;

	for (;;) {
		proc = remfirstq(&(eventtab[event]));
		if (proc == NULL)
			break;
		proc->state = PS_READY;
		insq(proc, &ready);
	}
	enable;
	return 0;
}

int event_uninst(int event, proc_t proc)
{
	if (event < 0 || event >= EVENTS || proc == NULL)
		return EINVAL;

	disable;

	if (proc->q != &(eventtab[event])) {
		enable;
		return EINVAL;
	}
	remq(proc, &(eventtab[event]));
	proc->state = PS_READY;

	enable;
	return 0;
}

int event_wait(int event)
{
	return event_inst(event, NULL);
}
