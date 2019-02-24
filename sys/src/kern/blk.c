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
#include <string.h>
#include <sys.h>
#include <sys/buf.h>
#include <sys/config.h>
#include <sys/mem.h>

struct blkpool blkpool;

void
blkpool_init(int size, int nblks)
{
    blk_t blk;
    int datasize, i;

    bzero(&blkpool, sizeof(struct blkpool));

    blkpool.blkbase = (char *) kmalloc(nblks * sizeof(struct blk));
    bzero(blkpool.blkbase, nblks * sizeof(struct blk));

    datasize = ALIGN(size, PAGE_SIZE);
    blkpool.database = (char *) kmalloc(nblks * datasize);
    bzero(blkpool.database, nblks * datasize);
    blkpool.size = datasize;

    for (i = 0; i < nblks; i++) {
	blk = (blk_t) (blkpool.blkbase + (i * sizeof(struct blk)));

	blk->index = i + 1;
	blk->blkpool = &blkpool;
	blk->data = blkpool.database + (i * datasize);
	blk_push(blk);
    }
}

void
blk_clear(blk_t blk)
{
    blk->next = NULL;
    blk->refcnt = 0;
    bzero(blk->data, blkpool.size);
}

void
blk_push(blk_t blk)
{
    blk_clear(blk);
    blk->next = blkpool.blks;
    blkpool.blks = blk;
    blkpool.nblks++;
}

blk_t
blk_pop()
{
    blk_t blk;

    if (blkpool.blks == NULL)
	return NULL;

    blk = blkpool.blks;
    blkpool.blks = blk->next;
    blk->next = NULL;
    blkpool.nblks--;
    return blk;
}
