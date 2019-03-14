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
#include <sys/intr.h>
#include <sys/mutex.h>
#include <sys/queue.h>

void mutex_clear(mutex_t mutex)
{
	mutex->holder = NULL;
	initq(&(mutex->waitq));
}

int mutex_lock(mutex_t mutex)
{
	disable;

	if (mutex->holder == current) {
		enable;
		return 0;
	} else if (mutex->holder == NULL) {
		mutex->holder = current;
		enable;
		return 0;
	}
	current->state = PS_MUTEX;
	insq(current, &(mutex->waitq));
	proc_transfer();	/* enables interrupts */
	return 0;
}

int mutex_trylock(mutex_t mutex)
{
	disable;

	if (mutex->holder == current) {
		enable;
		return 0;
	} else if (mutex->holder == NULL) {
		mutex->holder = current;
		enable;
		return 0;
	}
	enable;
	return EBUSY;
}

int mutex_unlock(mutex_t mutex)
{
	disable;

	if (mutex->holder != current) {
		enable;
		return EPERM;
	}
	mutex->holder = remfirstq(&(mutex->waitq));
	if (mutex->holder == NULL) {
		enable;
		return 0;
	}
	mutex->holder->state = PS_READY;
	insq(mutex->holder, &ready);
	enable;
	return 0;
}
