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
#include <fcntl.h>
#include <fs/rrfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int rrfile_unlink(char *path)
{
	file_t file = NULL;
	rrfs_t rrfs;
	rrfile_t rrfile;
	u_long clust, nextclust;
	u_long clustsize, off;
	int result;

	result = file_open(path, O_RDONLY, &file);
	if (result < 0)
		return result;

	if (file->flags & F_DIR) {
		file_close(file);
		return EISDIR;
	}
	rrfs = (rrfs_t) file->fs->data;
	rrfile = (rrfile_t) file->data;

	/* First file buffer is not needed */
	if (file->buf != NULL) {
		brel(file->buf);
		file->buf = NULL;
	}

	/* Clear cluster chain in the fat */
	for (clust = rrfile->firstclust;;) {

		/* Get fat block associated with cluster number */
		if ((result = rrfs_readfatblk(file->fs, clust)) < 0) {
#if _DEBUG
			kprintf("rrfile_unlink: read fatblk failed (%s)\n",
				strerror(result));
#endif
			goto unlink_done;
		}
		/* Get next cluster number */
		nextclust = rrfs->fatblk[clust % FAT_BLK_ENTRIES];

		/* Clear current cluster number */
		rrfs->fatblk[clust % FAT_BLK_ENTRIES] = 0;
		rrfs->flags |= RFS_FATBLK_DIRTY;

		if (nextclust >= rrfs->mbr->params.clusters)
			break;
		clust = nextclust;
	}
	/* Clear directory entry */
	clustsize = rrfs->mbr->params.sectorsperclust * SECTOR_SIZE;
#if _DEBUG
	kprintf("rrfile_unlink: clustsize %d declust %u deoff %u\n",
		clustsize, rrfile->declust, rrfile->deoff);
#endif

	/*
	 * Find the directory cluster containing the directory entry and adjust
	 * the offset to be from the beginning of that cluster
	 */
	for (clust = rrfile->declust, off = rrfile->deoff;
	     off >= clustsize;
	     clust = rrfs_nextclust(file->fs, clust), off -= clustsize) ;
#if _DEBUG
	kprintf("rrfile_unlink: directory entry clust %u off %u\n", clust, off);
#endif

	/* file->buf holds the cluster containing the directory entry */
	file->buf = bget(clustsize);
	blen(file->buf) = clustsize;

	result = rrfs_readclust(file, clust, &(file->buf));
	if (result < 0) {
#if _DEBUG
		kprintf
		    ("rrfile_unlink: read directory entry clust failed (%s)\n",
		     strerror(result));
#endif
		brel(file->buf);
		file->buf = NULL;
		goto unlink_done;
	}
#if _DEBUG
	rrfs_direntrydump((direntry_t) (bstart(file->buf) + off));
#endif
	bzero(bstart(file->buf) + off, DE_SIZE);
	((u_char *) bstart(file->buf))[off] = DE_DELETED;

	result = rrfs_writeclust(file, clust, &(file->buf));
	if (result < 0) {
#if _DEBUG
		kprintf
		    ("rrfile_unlink: write directory entry clust failed (%s)\n",
		     strerror(result));
#endif
	}

 unlink_done:
	file_close(file);
	return result;
}
