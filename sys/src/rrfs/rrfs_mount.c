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
#include <string.h>

int rrfs_mount(fs_t fs)
{
	rrfs_t rrfs;
	int rrfsno, result;

	mutex_lock(&rrfstabmutex);

	for (rrfsno = 0;
	     rrfsno < RR_FILE_SYSTEMS && rrfstab[rrfsno].mbrbuf != NULL;
	     rrfsno++) ;
	if (rrfsno == RR_FILE_SYSTEMS) {
		mutex_unlock(&rrfstabmutex);
		return EAGAIN;
	}
	rrfs = (rrfs_t) & (rrfstab[rrfsno]);
	fs->data = rrfs;

	if ((result = rrfs_readmbr(fs)) < 0) {
#if _DEBUG
		kprintf("rrfs_mount: failed (%s)\n", strerror(result));
#endif
		rrfs_clear(rrfs);
		mutex_unlock(&rrfstabmutex);
		return result;
	}
	mutex_unlock(&rrfstabmutex);
	return 0;
}
