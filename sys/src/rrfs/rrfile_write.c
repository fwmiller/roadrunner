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
#include <stdlib.h>

int rrfile_write(file_t file)
{
	rrfile_t rrfile = (rrfile_t) file->data;
	int result;

	if (file->buf == NULL)
		return EINVAL;

	if (rrfile->flags & RF_NEEDCLUST) {
		if (rrfile->currclust == FAT_CHAIN_END) {
			rrfile->firstclust = rrfs_clustalloc(file->fs);
#if _DEBUG
			kprintf("rrfile_write: alloc cluster %u\n",
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
			    ("rrfile_write: append cluster %u after %u\n",
			     rrfile->currclust, prevclust);
#endif
		}

		if (rrfile->currclust == FAT_CHAIN_END)
			return ENOSPC;
		rrfile->flags &= ~RF_NEEDCLUST;
	}
	result = rrfs_writeclust(file, rrfile->currclust, &(file->buf));
	if (result < 0)
		return result;

	/* Update directory containing file */
	result = rrfs_updatedir(file, rrfile->firstclust);
	if (result < 0) {
		file->flags |= F_ERR;
		return result;
	}
	/* Another cluster needs to be added to the file */
	rrfile->flags |= RF_NEEDCLUST;

	return 0;
}
