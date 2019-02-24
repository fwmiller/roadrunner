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

#include <dev.h>
#include <errno.h>
#include <fs/rrfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mem.h>

#define RRFS_STRATEGY_READ	0
#define RRFS_STRATEGY_WRITE	1

/*
 * Transfer a buffer from/to disk.  The routine assumes that fs->blkno
 * contains the address of the block to be transferred.
 */
static int
rrfs_strategy(fs_t fs, buf_t * b, int direction)
{
    struct seek seekargs;
    int result;

    /* Lock device */
    if ((result = dev_ioctl(fs->devno, LOCK, NULL)) < 0) {
#if _DEBUG
	kprintf("rrfs_strategy: device lock failed (%s)\n",
		strerror(result));
#endif
	return result;
    }
    /* Seek to disk sector */
    seekargs.offset = fs->blkno;
    seekargs.whence = SEEK_SET;
    if ((result = dev_ioctl(fs->devno, SEEK_BLOCK, &seekargs)) < 0) {
#if _DEBUG
	kprintf("rrfs_strategy: seek failed (%s)\n", strerror(result));
#endif
	dev_ioctl(fs->devno, UNLOCK, NULL);
	return result;
    }
    /* Perform transfer */
    if (direction == RRFS_STRATEGY_WRITE)
	result = dev_write(fs->devno, b);
    else
	result = dev_read(fs->devno, b);

    if (result < 0) {
#if _DEBUG
	kprintf("rrfs_strategy: ");
	if (direction == RRFS_STRATEGY_WRITE)
	    kprintf("write");
	else
	    kprintf("read");
	kprintf(" failed (%s)\n", strerror(result));
#endif
	dev_ioctl(fs->devno, UNLOCK, NULL);
	return result;
    }
    dev_ioctl(fs->devno, UNLOCK, NULL);
    return 0;
}

/*
 * Read a single sector of data into a buffer.  The routine assumes that
 * fs->blkno contains the address of the block to be transferred.
 */
static int
rrfs_readsector(fs_t fs, buf_t * b)
{
    blen(*b) = SECTOR_SIZE;
    return rrfs_strategy(fs, b, RRFS_STRATEGY_READ);
}

/* Read a cluster of data into a buffer */
int
rrfs_readclust(file_t file, u_long clust, buf_t * b)
{
    rrfs_t rrfs = (rrfs_t) file->fs->data;

    if (clust >= rrfs->mbr->params.clusters)
	return EINVAL;

    file->fs->blkno = CLUST2BLKNO(rrfs, clust);
    return rrfs_strategy(file->fs, b, RRFS_STRATEGY_READ);
}

int
rrfs_readmbr(fs_t fs)
{
    rrfs_t rrfs = (rrfs_t) fs->data;
    buf_t b;
    int result;

    if (rrfs->mbrbuf == NULL) {
	rrfs->mbrbuf = kmalloc(SECTOR_SIZE);
	if (rrfs->mbrbuf == NULL)
	    return ENOMEM;
	rrfs->mbr = (mbr_t) rrfs->mbrbuf;
    }
    bzero(rrfs->mbrbuf, SECTOR_SIZE);

    b = bget(SECTOR_SIZE);
    blen(b) = SECTOR_SIZE;
    fs->blkno = 0;
    if ((result = rrfs_readsector(fs, &b)) < 0) {
#if _DEBUG
	kprintf("rrfs_readmbr: read sector failed (%s)\n",
		strerror(result));
#endif
	brel(b);
	rrfs->mbr = NULL;
	kfree(rrfs->mbrbuf);
	rrfs->mbrbuf = NULL;
	return result;
    }
    bcopy(bstart(b), rrfs->mbrbuf, SECTOR_SIZE);
    brel(b);

    if (rrfs->mbr->params.bytespersector != SECTOR_SIZE) {
#if _DEBUG
	kprintf("rrfs_readmbr: not an rrfs file system\n");
#endif
	rrfs->mbr = NULL;
	kfree(rrfs->mbrbuf);
	rrfs->mbrbuf = NULL;
	return EBADFS;
    }
#if _DEBUG
    kprintf("rrfs_readmbr: ");
    kprintf("%u tracks %u heads %u sec/trk %u sectors\n",
	    (u_int) rrfs->mbr->params.tracks,
	    (u_int) rrfs->mbr->params.heads,
	    (u_int) rrfs->mbr->params.sectorspertrack,
	    (u_int) rrfs->mbr->params.sectors);
    kprintf("rrfs_readmbr: ");
    kprintf("%u sec/fat %u sec/clust %u clusters\n",
	    (u_int) rrfs->mbr->params.fatsectors,
	    (u_int) rrfs->mbr->params.sectorsperclust,
	    (u_int) rrfs->mbr->params.clusters);
#endif
    return 0;
}

int
rrfs_readfatblk(fs_t fs, u_long clust)
{
    rrfs_t rrfs = (rrfs_t) fs->data;
    int result;

    if (rrfs->mbr == NULL)
	return EINVAL;

    if (rrfs->fatbuf != NULL && (result = rrfs_writefatblk(fs)) < 0) {
#if _DEBUG
	kprintf("rrfs_readfatblk: write fat block failed (%s)\n",
		strerror(result));
#endif
	return result;
    }
    rrfs->fatbuf = bget(SECTOR_SIZE);
    blen(rrfs->fatbuf) = SECTOR_SIZE;
    fs->blkno = FAT_BLKNO(clust) + rrfs->mbr->params.bootsectors;
    if ((result = rrfs_readsector(fs, &(rrfs->fatbuf))) < 0) {
#if _DEBUG
	kprintf("rrfs_readfatblk: read sector failed (%s)\n",
		strerror(result));
#endif
	brel(rrfs->fatbuf);
	rrfs->fatbuf = NULL;
	return result;
    }
    rrfs->fatblkno = fs->blkno;
    rrfs->fatblk = (u_long *) bstart(rrfs->fatbuf);
    return 0;
}

/*
 * Write a single sector of data to disk.  The routine assumes that
 * fs->blkno contains the address of the block to be transferred.
 */
static int
rrfs_writesector(fs_t fs, buf_t * b)
{
    blen(*b) = SECTOR_SIZE;
    return rrfs_strategy(fs, b, RRFS_STRATEGY_WRITE);
}

/* Write a cluster of data to disk */
int
rrfs_writeclust(file_t file, u_long clust, buf_t * b)
{
    rrfs_t rrfs = (rrfs_t) file->fs->data;

    file->fs->blkno = CLUST2BLKNO(rrfs, clust);
    blen(*b) = file->bufsize;
    return rrfs_strategy(file->fs, b, RRFS_STRATEGY_WRITE);
}

int
rrfs_writembr(fs_t fs)
{
    rrfs_t rrfs = (rrfs_t) fs->data;
    buf_t b;
    int result;

    if (rrfs->mbr == NULL)
	return EINVAL;

    b = bget(SECTOR_SIZE);
    blen(b) = SECTOR_SIZE;
    bcopy(rrfs->mbrbuf, bstart(b), SECTOR_SIZE);
    fs->blkno = 0;
    if ((result = rrfs_writesector(fs, &b)) < 0) {
#if _DEBUG
	kprintf("rrfs_writembr: write sector failed (%s)\n",
		strerror(result));
#endif
	brel(b);
	return result;
    }
    return 0;
}

int
rrfs_writefatblk(fs_t fs)
{
    rrfs_t rrfs = (rrfs_t) fs->data;
    int result;

    if (rrfs->fatbuf == NULL)
	return EINVAL;

    if (rrfs->flags & RFS_FATBLK_DIRTY) {
	fs->blkno = rrfs->fatblkno;
	if ((result = rrfs_writesector(fs, &(rrfs->fatbuf))) < 0) {
#if _DEBUG
	    kprintf
		("rrfs_writefatblk: write sector failed (%s)\n",
		 strerror(result));
#endif
	}
	rrfs->fatblk = NULL;
    } else {
	brel(rrfs->fatbuf);
	rrfs->fatbuf = NULL;
    }
    return 0;
}
