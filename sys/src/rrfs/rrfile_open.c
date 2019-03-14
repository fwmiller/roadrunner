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

int rrfile_open(file_t file)
{
	rrfs_t rrfs = (rrfs_t) file->fs->data;
	rrfile_t rrfile;
	int rrfileno, result;

	mutex_lock(&rrfiletabmutex);

	for (rrfileno = 0;
	     rrfileno < RR_FILES && rrfiletab[rrfileno].flags & RF_INUSE;
	     rrfileno++) ;
	if (rrfileno == RR_FILES) {
#if _DEBUG
		kprintf("rrfile_open: rrfiletab full\n");
#endif
		mutex_unlock(&rrfiletabmutex);
		return EAGAIN;
	}
	rrfile = (rrfile_t) & (rrfiletab[rrfileno]);
	rrfile->flags |= RF_INUSE;
	mutex_unlock(&rrfiletabmutex);

	file->data = rrfile;
	file->bufsize =
	    rrfs->mbr->params.sectorsperclust *
	    rrfs->mbr->params.bytespersector;

	if (file->flags & O_RDONLY) {
		/* Look for an existing file */
		if ((result = rrfs_lookup(file, file->path)) < 0)
			goto openerror;

		if (file->filesize == 0 && !(file->flags & F_DIR)) {
			/* 
			 * The file has a directory entry but a zero file length and is
			 * not itself a directory.  Definitely at eof and no need to read
			 * first cluster.
			 */
			file->flags |= F_EOF;
			return 0;
		}
		if (rrfile->firstclust >= rrfs->mbr->params.clusters) {
#if _DEBUG
			kprintf
			    ("rrfile_open: read only illegal first cluster\n");
#endif
			result = EBADCLUST;
			goto openerror;
		}
		/* Get buffer for first cluster */
		if (file->buf != NULL)
			brel(file->buf);
		file->buf = bget(file->bufsize);
		blen(file->buf) = file->bufsize;

		/* Read first cluster */
		result = rrfs_readclust(file, rrfile->firstclust, &(file->buf));
		if (result < 0) {
#if _DEBUG
			kprintf
			    ("rrfile_open: read first cluster failed (%s)\n",
			     strerror(result));
#endif
			goto openerror;
		}
		return 0;

	} else if (file->flags & O_WRONLY || file->flags & O_RDWR) {
		file->flags |= F_EOF;

		if ((result = rrfs_lookup(file, file->path)) == 0) {
			u_long prevclust;

			if (file->filesize == 0 && !(file->flags & F_DIR)) {
				/* 
				 * The file has a directory entry but a zero file length and
				 * is not itself a directory.  Definitely at eof and no need
				 * to read last cluster.
				 */
				file->pos = 0;
				rrfile->flags |= RF_NEEDCLUST;
				return 0;
			}
			/* Scan to the last cluster */
			for (rrfile->currclust = rrfile->firstclust;;) {
				if (rrfile->currclust >=
				    rrfs->mbr->params.clusters)
					break;
				prevclust = rrfile->currclust;
				rrfile->currclust =
				    rrfs_nextclust(file->fs, rrfile->currclust);
			}
			rrfile->currclust = prevclust;

			if (rrfile->currclust >= rrfs->mbr->params.clusters) {
#if _DEBUG
				kprintf("rrfile_open: ");
				kprintf
				    ("write only or read/write illegal first cluster\n");
#endif
				result = EBADCLUST;
				goto openerror;
			}
			if (file->filesize % file->bufsize == 0) {
				/* 
				 * Special case when file size is multiple of the block size.
				 * Don't allocate a buffer and we'll need a cluster the first
				 * time a buffer is written.
				 */
				file->pos = file->filesize;
				rrfile->flags |= RF_NEEDCLUST;
				return 0;
			}
			/* Get buffer for last cluster */
			if (file->buf != NULL)
				brel(file->buf);
			file->buf = bget(file->bufsize);
			blen(file->buf) = file->bufsize;

			/* Read last cluster */
			result =
			    rrfs_readclust(file, rrfile->currclust,
					   &(file->buf));
			if (result < 0) {
#if _DEBUG
				kprintf
				    ("rrfile_open: read last cluster failed (%s)\n",
				     strerror(result));
#endif
				goto openerror;
			}
			file->pos = file->filesize;
			bpos(file->buf) = file->filesize % file->bufsize;
			return 0;

		} else if (file->flags & O_CREAT) {
			result =
			    rrfile_create(file,
					  (file->flags & O_MKDIR ? 1 : 0));
			if (result < 0) {
#if _DEBUG
				kprintf
				    ("rrfile_open: file create failed (%s)\n",
				     strerror(result));
#endif
				goto openerror;
			}
			rrfile->flags |= RF_NEEDCLUST;
			file->pos = 0;

			if (file->flags & O_MKDIR) {
				file->buf = bget(file->bufsize);
				blen(file->buf) = file->bufsize;
				bzero(bstart(file->buf), file->bufsize);
			}
			return 0;

		} else {
			result = ENOENT;
			goto openerror;
		}
	}
	/* Mode error */
	result = EINVAL;

 openerror:
	mutex_lock(&rrfiletabmutex);
	rrfile_clear(rrfile);
	mutex_unlock(&rrfiletabmutex);
	return result;
}
