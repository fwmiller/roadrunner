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
#include <fs/rrfs.h>
#include <stdio.h>
#include <string.h>

int rrfile_read(file_t file)
{
	rrfs_t rrfs = (rrfs_t) file->fs->data;
	rrfile_t rrfile = (rrfile_t) file->data;

#if _DEBUG
	u_long prevclust;
#endif
	int result;

	if (file->flags & F_EOF)
		return EINVAL;

#if _DEBUG
	prevclust = rrfile->currclust;
#endif
	rrfile->currclust = rrfs_nextclust(file->fs, rrfile->currclust);
	if (rrfile->currclust >= rrfs->mbr->params.clusters) {
#if _DEBUG
		kprintf("rrfile_read: bad cluster %u prev %u\n",
			rrfile->currclust, prevclust);
#endif
		file->flags |= F_ERR;
		return EBADCLUST;
	}

	/* Assume fs layer has provided a buffer */

	result = rrfs_readclust(file, rrfile->currclust, &(file->buf));
	if (result < 0) {
#if _DEBUG
		kprintf("rrfile_read: read cluster failed (%s)\n",
			strerror(result));
#endif
		file->flags |= F_ERR;
	}
	return result;
}
