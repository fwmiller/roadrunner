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

int
rrfile_close(file_t file)
{
    rrfile_t rrfile = (rrfile_t) file->data;
    int result;

    if (file->flags & F_ERR)
	return EINVAL;

    if (file->flags & O_RDONLY && file->buf != NULL) {
	brel(file->buf);
	file->buf = NULL;

    } else if (file->flags & O_WRONLY || file->flags & O_RDWR) {
	if (file->buf != NULL) {
	    if (rrfile->flags & RF_NEEDCLUST) {
		if (rrfile->firstclust == FAT_CHAIN_END) {
		    rrfile->firstclust = rrfs_clustalloc(file->fs);
#if _DEBUG
		    kprintf
			("rrfile_close: alloc cluster %u\n",
			 rrfile->firstclust);
#endif
		    rrfile->currclust = rrfile->firstclust;
		} else {
#if _DEBUG
		    u_long prevclust = rrfile->currclust;
#endif
		    rrfile->currclust =
			rrfs_clustappend(file->fs, rrfile->currclust);
#if _DEBUG
		    kprintf
			("rrfile_close: append cluster %u after %u\n",
			 rrfile->currclust, prevclust);
#endif
		}

		if (rrfile->currclust == FAT_CHAIN_END) {
#if _DEBUG
		    kprintf("rrfile_close: file system full\n");
#endif
		    return ENOSPC;
		}
		rrfile->flags &= ~RF_NEEDCLUST;
	    }
	    result = rrfs_writeclust(file, rrfile->currclust, &(file->buf));
	    if (result < 0) {
#if _DEBUG
		kprintf
		    ("rrfile_close: cluster write failed (%s)\n",
		     strerror(result));
#endif
		file->flags |= F_ERR;
		return result;
	    }
	}
	/* Update directory containing closed file */
	result = rrfs_updatedir(file, rrfile->firstclust);
	if (result < 0) {
	    file->flags |= F_ERR;
	    return result;
	}
    }
    mutex_lock(&rrfiletabmutex);
    rrfile_clear(rrfile);
    mutex_unlock(&rrfiletabmutex);

    return 0;
}
