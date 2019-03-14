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
#include <sys/buf.h>

void binitq(bufq_t q)
{
	if (q == NULL)
		return;

	q->len = 0;
	q->h = q->t = NULL;
}

int blenq(bufq_t q)
{
	if (q == NULL)
		return 0;

	return q->len;
}

void benq(buf_t b, bufq_t q)
{
	if (q == NULL)
		return;

	if (q->t == NULL)
		q->h = q->t = b;
	else {
		q->t->next = b;
		b->prev = q->t;
		q->t = b;
	}
	q->len++;
}

buf_t bdeq(bufq_t q)
{
	buf_t b;

	if (q == NULL || q->h == NULL)
		return NULL;

	b = q->h;
	q->h = b->next;
	if (q->h == NULL)
		q->t = NULL;
	else
		q->h->prev = NULL;
	b->prev = NULL;
	b->next = NULL;
	q->len--;
	return b;
}

void bremq(buf_t b, bufq_t q)
{
	buf_t p;

	if (q == NULL)
		return;

	/* Special case for b at the head of q */
	if (q->h == b) {
		bdeq(q);
		return;
	}
	/* b is not at the head of q */
	for (p = q->h; p->next != NULL; p = p->next)
		if (p->next == b) {
			p->next = b->next;
			if (p->next == NULL)
				q->t = p;
			else
				p->next->prev = p;
			b->prev = b->next = NULL;
			q->len--;
			break;
		}
}
