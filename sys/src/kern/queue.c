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

#include <stdlib.h>
#include <sys/queue.h>

void
initq(queue_t q)
{
    q->h = NULL;
    q->t = NULL;
}

proc_t
firstq(queue_t q)
{
    return q->h;
}

proc_t
remfirstq(queue_t q)
{
    proc_t p = q->h;

    if (p == NULL)
	return NULL;

    if (p->next == NULL)
	q->h = q->t = NULL;
    else
	q->h = p->next;

    p->next = NULL;
    return p;
}

void
remq(proc_t p, queue_t q)
{
    proc_t prev, curr;

    for (prev = NULL, curr = q->h; curr != NULL;
	 prev = curr, curr = curr->next) {
	if (curr == p) {
	    if (prev == NULL)
		q->h = curr->next;
	    else
		prev->next = curr->next;

	    if (curr == q->t)
		q->t = prev;

	    p->next = NULL;
	    return;
	}
    }
}

void
insq(proc_t p, queue_t q)
{
    if (q->t == NULL)
	q->h = p;
    else
	q->t->next = p;
    q->t = p;
}
