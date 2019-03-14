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
#include <sys/time.h>

int rrfs_updatedir(file_t file, u_long firstclust)
{
	rrfile_t rrfile = (rrfile_t) file->data;
	buf_t b;
	u_int clust;
	direntry_t de;
	time_t t;
	struct tm tm;
	int off, result;

	/* 
	 * Scan directory clusters looking for the one that contains the
	 * directory entry we're interested in
	 */
	for (clust = rrfile->declust, off = rrfile->deoff;
	     off >= file->bufsize;
	     clust = rrfs_nextclust(file->fs, clust), off -= file->bufsize) ;

	/* Read the cluster */
	b = bget(file->bufsize);
	blen(b) = file->bufsize;
	if ((result = rrfs_readclust(file, clust, &b)) < 0) {
#if _DEBUG
		kprintf("rrfs_updatedir: read cluster failed (%s)\n",
			strerror(result));
#endif
		brel(b);
		return result;
	}
	de = (direntry_t) (bstart(b) + off);

	/* Update time and date */
	t = time();
	time2tm(t, &tm);
	de->time[0] = (u_char) tm.tm_hour;
	de->time[1] = (u_char) tm.tm_min;
	de->time[2] = (u_char) tm.tm_sec;
	de->date[0] = (u_char) tm.tm_mon;
	de->date[1] = (u_char) tm.tm_mday;
	de->date[2] = (u_char) tm.tm_year;

	/* Update file size and first cluster */
	de->size = (u_long) file->pos;
	de->start = (u_long) firstclust;

	/* Write the updated cluster */
	return rrfs_writeclust(file, clust, &b);
}
