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
#include <sys.h>
#include <sys/mem.h>
#include <sys/time.h>

static void rrfs_deinit(direntry_t de, char *fname, int directory)
{
	time_t t;
	struct tm tm;

	bzero(de, sizeof(struct direntry));

	strncpy(de->name, fname, DE_NAME_LENGTH);
	de->attr = (DE_ATTR_READ | DE_ATTR_WRITE);
	if (directory)
		de->attr |= DE_ATTR_DIR;

	t = time();
	time2tm(t, &tm);
	de->time[DE_TIME_HOUR] = tm.tm_hour;
	de->time[DE_TIME_MIN] = tm.tm_min;
	de->time[DE_TIME_SEC] = tm.tm_sec;
	de->date[DE_DATE_MON] = tm.tm_mon;
	de->date[DE_DATE_DAY] = tm.tm_mday;
	de->date[DE_DATE_YEAR] = tm.tm_year;
}

int rrfile_create(file_t file, int directory)
{
	file_t tmpfile = file;
	rrfile_t rrfile;
	char *dir, *fulldir, *fname;
	u_long declust, deoff;
	int found, result;
	direntry_t de;

	dir = (char *)malloc(3 * PATH_LENGTH);
	if (dir == NULL)
		return ENOMEM;

	bzero(dir, 3 * PATH_LENGTH);
	fulldir = dir + PATH_LENGTH;
	fname = dir + 2 * PATH_LENGTH;

	getdir(file->path, dir);
	mkpath(file->fs->path, dir, fulldir);
	getname(file->path, fname);

	if ((result = file_open(fulldir, O_RDONLY, &file)) < 0) {
#if _DEBUG
		kprintf("rrfile_create: could not open %s (%s)\n",
			fulldir, strerror(result));
#endif
		goto createerror;
	}
	if (!(file->flags & F_DIR)) {
#if _DEBUG
		kprintf("rrfile_create: %s is not a directory\n", fulldir);
#endif
		file_close(file);
		result = ENOTDIR;
		goto createerror;
	}
	rrfile = (rrfile_t) file->data;

	/* This is directory entry data for the new file */
	declust = rrfile->firstclust;
	deoff = 0;

	/* Loop over the directory entries looking for a free slot */
	for (rrfile->currclust = rrfile->firstclust, found = 0;;) {
		u_int clust;

		for (bpos(file->buf) = 0;
		     bpos(file->buf) < file->bufsize;
		     bpos(file->buf) += DE_SIZE) {
			u_char ch;

			ch = *((u_char *)
			       (bstart(file->buf) + bpos(file->buf)));
			if (ch == (u_char) DE_UNUSED
			    || ch == (u_char) DE_DELETED) {
				found = 1;
				break;
			}
		}
		if (found)
			break;
		deoff += file->bufsize;

		if (file->buf != NULL)
			brel(file->buf);
		file->buf = bget(file->bufsize);
		blen(file->buf) = file->bufsize;

		clust = rrfs_nextclust(file->fs, rrfile->currclust);
		if (clust == FAT_CHAIN_END) {
			/* 
			 * Assume we've filled up the last cluster in the directory chain
			 * and need to add a new cluster to the end
			 */
			clust = rrfs_clustappend(file->fs, rrfile->currclust);
			if (clust == FAT_CHAIN_END) {
#if _DEBUG
				kprintf
				    ("rrfile_create: cluster append failed\n");
#endif
				file_close(file);
				result = ENOSPC;
				goto createerror;
			}
			rrfile->currclust = clust;
			result =
			    rrfs_readclust(file, rrfile->currclust,
					   &(file->buf));
			if (result < 0) {
#if _DEBUG
				kprintf
				    ("rrfile_create: read cluster failed (%s)\n",
				     strerror(result));
#endif
				file_close(file);
				goto createerror;
			}
			bzero(bstart(file->buf), file->bufsize);
			continue;
		}
		rrfile->currclust = clust;
		result = rrfs_readclust(file, rrfile->currclust, &(file->buf));
		if (result < 0) {
#if _DEBUG
			kprintf("rrfile_create: read cluster failed (%s)\n",
				strerror(result));
#endif
			file_close(file);
			goto createerror;
		}
	}
	deoff += bpos(file->buf);

	/* Initialize directory entry fields */
	de = (direntry_t) (bstart(file->buf) + bpos(file->buf));
	rrfs_deinit(de, fname, directory);

	/* Write directory entry */
	result = rrfs_writeclust(file, rrfile->currclust, &(file->buf));
	if (result < 0) {
#if _DEBUG
		kprintf("rrfile_create: write cluster failed (%s)\n",
			strerror(result));
#endif
		file_close(file);
		goto createerror;
	}
	file_close(file);
	file = tmpfile;
	rrfile = (rrfile_t) file->data;
	rrfile->declust = declust;
	rrfile->deoff = deoff;

 createerror:
	free(dir);
	return result;
}
