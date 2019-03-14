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

static int next_buffer(file_t file)
{
	rrfs_t rrfs = (rrfs_t) file->fs->data;
	rrfile_t rrfile = (rrfile_t) file->data;
	int result;

	rrfile->currclust = rrfs_nextclust(file->fs, rrfile->currclust);
	if (rrfile->currclust >= rrfs->mbr->params.clusters)
		return EFILEEOF;
	file->buf = bget(file->bufsize);
	blen(file->buf) = file->bufsize;
	result = rrfs_readclust(file, rrfile->currclust, &(file->buf));
#if _DEBUG
	if (result < 0)
		kprintf("next_buffer: read cluster failed (%s)\n",
			strerror(result));
#endif
	return result;
}

/* 
 * Roadrunner file system group member attributes include the following:
 *
 * string[4]    Attributes
 * u_int        Size
 * string[22]   Last modified date and time
 * string[48]   Name
 */
struct gmentry {
	char attr[4];
	u_int size;
	char time[22];
	char name[48];
} __attribute__ ((packed));

typedef struct gmentry *gmentry_t;

int rrfile_readdir(file_t file, char *entry)
{
	direntry_t de;
	gmentry_t gm;
	int hour, result;

	if (file->buf == NULL)
		if ((result = next_buffer(file)) < 0) {
#if _DEBUG
			kprintf
			    ("rrfile_readdir: could not get next cluster (%s)\n",
			     strerror(result));
#endif
			return result;
		}

	for (de = (direntry_t) (bstart(file->buf) + bpos(file->buf));
	     ((u_char) de->name[0]) == DE_UNUSED ||
	     ((u_char) de->name[0]) == DE_DELETED;) {
		bpos(file->buf) += DE_SIZE;
		if (bpos(file->buf) >= file->bufsize) {
			brel(file->buf);
			file->buf = NULL;
			if ((result = next_buffer(file)) < 0) {
#if _DEBUG
				kprintf
				    ("rrfile_readdir: could not get next cluster (%s)\n",
				     strerror(result));
#endif
				return result;
			}
		}
		de = (direntry_t) (bstart(file->buf) + bpos(file->buf));
	}

#if _DEBUG
	rrfs_direntrydump(de);
#endif
	/* Setup attributes string */
	bzero(entry, sizeof(struct gmentry));

	gm = (gmentry_t) entry;
	gm->attr[0] = (de->attr & DE_ATTR_DIR ? 'd' : '-');
	gm->attr[1] = (de->attr & DE_ATTR_READ ? 'r' : '-');
	gm->attr[2] = (de->attr & DE_ATTR_WRITE ? 'w' : '-');
	gm->attr[3] = (de->attr & DE_ATTR_EXEC ? 'x' : '-');

	/* File size */
	gm->size = (u_int) de->size;

	/* Setup date and time string */
	bzero(gm->time, 22);
	hour = (int)de->time[DE_TIME_HOUR];
	if (hour == 0)
		hour = 24;
	sprintf(gm->time, " %2d", (hour > 12 ? hour - 12 : hour));

	sprintf(gm->time + 3, ":%02d:%02d",
		(int)de->time[DE_TIME_MIN], (int)de->time[DE_TIME_SEC]);

	if (hour < 12)
		sprintf(gm->time + 9, "a");
	else
		sprintf(gm->time + 9, "p");

	sprintf(gm->time + 10, " %2d-%02d-%04d",
		(int)de->date[DE_DATE_MON] + 1,
		(int)de->date[DE_DATE_DAY], (int)de->date[DE_DATE_YEAR] + 1900);

	/* File name */
	strcpy(gm->name, de->name);

	if ((bpos(file->buf) += DE_SIZE) >= file->bufsize) {
		brel(file->buf);
		file->buf = NULL;
	}
	return 0;
}
