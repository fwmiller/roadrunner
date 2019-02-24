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

#ifndef __BUF_H
#define __BUF_H

#if _KERNEL

#include <sys/types.h>

/* buf abbreviations */
#define bdata(B) ((B)->blk->data)
#define bsize(B) (((blkpool_t) ((B)->blk->blkpool))->size)
#define bstart(B) ((B)->start)
#define blen(B) ((B)->len)
#define bpos(B) ((B)->pos)

typedef struct blk {
    struct blk *next;
    void *blkpool;		       /* Owning blk pool */
    int index;			       /* Index of blk in blk pool */
    int refcnt;			       /* Buf reference count */
    char *data;			       /* Data */
} *blk_t;

typedef struct blkpool {
    int nblks;			       /* Number of blks in pool */
    int size;			       /* Block size */
    blk_t blks;			       /* List of blks */
    char *blkbase;		       /* Block desc array base addr */
    char *database;		       /* Block data array base addr */
} *blkpool_t;

typedef struct buf {
    struct buf *prev, *next;
    char *start;		       /* Start ptr in blk */
    int len;			       /* Data len in blk */
    int pos;			       /* Current position in blk */
    blk_t blk;			       /* Blk ptr */
} *buf_t;

typedef struct bufpool {
    int total;
    int nbufs;
    buf_t bufs;
    char *base;
} *bufpool_t;

typedef struct bufq {
    int len;			       /* Number of buffers in queue */
    buf_t h, t;			       /* Head and tail ptrs */
} *bufq_t;

extern struct blkpool blkpool;
extern struct bufpool bufpool;

void blkpool_init(int size, int nblks);
void blk_clear(blk_t blk);
void blk_push(blk_t blk);
blk_t blk_pop();

void bufpool_init(int nbufs);
void buf_clear(buf_t buf);
void buf_push(buf_t buf);
buf_t buf_pop();
buf_t _balloc();
buf_t balloc();
void _bfree(buf_t buf);
int bfree(buf_t buf);
buf_t _bget(int size);
buf_t bget(int size);
int brel(buf_t buf);

void binitq(bufq_t q);
int blenq(bufq_t q);
void benq(buf_t b, bufq_t q);
buf_t bdeq(bufq_t q);
void bremq(buf_t b, bufq_t q);

#endif				/* _KERNEL */

#endif
