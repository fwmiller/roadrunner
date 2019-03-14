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
#include <event.h>
#include <stdlib.h>
#include <string.h>
#include <sys/buf.h>
#include <sys/intr.h>
#include <sys/mem.h>

struct bufpool bufpool;

void bufpool_init(int nbufs)
{
	int i;

	bzero(&bufpool, sizeof(struct bufpool));

	bufpool.base = (char *)kmalloc(nbufs * sizeof(struct buf));
	bzero(bufpool.base, nbufs * sizeof(struct buf));

	for (i = 0; i < nbufs; i++)
		buf_push((buf_t) (bufpool.base + (i * sizeof(struct buf))));

	bufpool.total = nbufs;
}

void buf_clear(buf_t buf)
{
	buf->prev = NULL;
	buf->next = NULL;
	buf->start = NULL;
	buf->len = 0;
	buf->pos = 0;
	buf->blk = NULL;
}

void buf_push(buf_t buf)
{
	buf_clear(buf);
	buf->next = bufpool.bufs;
	bufpool.bufs = buf;
	bufpool.nbufs++;
}

buf_t buf_pop()
{
	buf_t buf;

	if (bufpool.bufs == NULL)
		return NULL;

	buf = bufpool.bufs;
	bufpool.bufs = buf->next;
	buf->next = NULL;
	bufpool.nbufs--;

	return buf;
}

buf_t _balloc()
{
	return buf_pop(bufpool);
}

buf_t balloc()
{
	buf_t buf;

	for (;;) {
		disable;
		buf = _balloc(bufpool);
		enable;
		if (buf != NULL)
			return buf;
		event_wait(EVENT_BUF);
	}
}

void _bfree(buf_t buf)
{
	if (buf->blk != NULL) {
		if (buf->blk->refcnt > 1)
			buf->blk->refcnt--;
		else {
			blk_clear(buf->blk);
			blk_push(buf->blk);
		}
	}
	buf_clear(buf);
	buf_push(buf);
}

int bfree(buf_t buf)
{
	if (buf == NULL)
		return EINVAL;

	disable;
	_bfree(buf);
	enable;

	event_raise(EVENT_BUF);
	return 0;
}

buf_t _bget(int size)
{
	buf_t buf;
	blk_t blk;

	disable;

	blk = blk_pop();
	if (blk == NULL) {
		enable;
		return NULL;
	}
	buf = _balloc();
	if (buf == NULL) {
		blk_push(blk);
		enable;
		return NULL;
	}
	blk->refcnt = 1;
	buf->blk = blk;
	buf->start = buf->blk->data;

	enable;
	return buf;
}

buf_t bget(int size)
{
	buf_t buf;

	while ((buf = _bget(size)) == NULL)
		event_wait(EVENT_BUF);
	return buf;
}

int brel(buf_t buf)
{
	return bfree(buf);
}
