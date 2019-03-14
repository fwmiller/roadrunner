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

#include <ctype.h>
#include <errno.h>
#include <fs/rrfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RRFS_FILENAMECHAR(C)						\
    (isalnum(C) || (C) == '_' || (C) == '.' || (C) == '-' ||		\
     (C) == '%' || (C) == '(' || (C) == ')' || (C) == '+' ||		\
     (C) == ',' || (C) == '$' || (C) == '#' || (C) == ':' ||		\
     (C) == ';' || (C) == '<' || (C) == '=' || (C) == '>' ||		\
     (C) == '?' || (C) == '@' || (C) == '[' || (C) == ']' ||		\
     (C) == '^' || (C) == '!' || (C) == '{' || (C) == '|' ||		\
     (C) == '}' || (C) == '~')

#define RRFS_VALIDCLUST(RRFS, CLUST)					\
    ((CLUST) < (RRFS)->mbr->params.clusters)

void rrfs_clear(rrfs_t rrfs)
{
	rrfs->flags = 0;
	rrfs->mbrbuf = NULL;
	rrfs->mbr = NULL;
	rrfs->fatblkno = 0;
	rrfs->fatbuf = NULL;
	rrfs->fatblk = NULL;
}

void rrfile_clear(rrfile_t rrfile)
{
	rrfile->flags = 0;
	rrfile->declust = FAT_CHAIN_END;
	rrfile->deoff = 0;
	rrfile->firstclust = FAT_CHAIN_END;
	rrfile->currclust = FAT_CHAIN_END;
}

void rrfs_nextpathelem(char *path, int *pos, char *name)
{
	int i;

	bzero(name, DE_NAME_LENGTH);
	if (path[*pos] == '/')
		(*pos)++;
	for (i = 0;
	     i < DE_NAME_LENGTH - 1 && RRFS_FILENAMECHAR(path[*pos]);
	     i++, (*pos)++)
		name[i] = path[*pos];
}

u_long rrfs_nextclust(fs_t fs, u_long clust)
{
	rrfs_t rrfs = (rrfs_t) fs->data;
	u_long newclust;
	int result;

	if (!RRFS_VALIDCLUST(rrfs, clust))
		return FAT_CHAIN_END;

	if ((result = rrfs_readfatblk(fs, clust)) < 0) {
#if _DEBUG
		kprintf("rrfs_nextclust: read fatblk failed (%s)\n",
			strerror(result));
#endif
		return FAT_CHAIN_END;
	}
	newclust = rrfs->fatblk[clust % FAT_BLK_ENTRIES];
	if (RRFS_VALIDCLUST(rrfs, newclust))
		return newclust;
#if _DEBUG
	kprintf("rrfs_nextclust: invalid cluster\n");
#endif
	return FAT_CHAIN_END;
}

static int rrfs_clustalloc1(rrfs_t rrfs)
{
	int entry;

	for (entry = 0; entry < FAT_BLK_ENTRIES; entry++)
		if (rrfs->fatblk[entry] == 0) {
			rrfs->fatblk[entry] = FAT_CHAIN_END;
			rrfs->flags |= RFS_FATBLK_DIRTY;
			return entry;
		}
	return ENOSPC;
}

u_long rrfs_clustalloc(fs_t fs)
{
	rrfs_t rrfs = (rrfs_t) fs->data;
	u_long clust;
	int entry, result;

	for (clust = 0;
	     clust < rrfs->mbr->params.clusters; clust += FAT_BLK_ENTRIES) {

		/* Read in fat block to be searched */
		if ((result = rrfs_readfatblk(fs, clust)) < 0) {
#if _DEBUG
			kprintf
			    ("rrfs_clustalloc: read fatblk failed (%s)\n",
			     strerror(result));
#endif
			return FAT_CHAIN_END;
		}
		/* Search fat block for a free cluster entry */
		entry = rrfs_clustalloc1(rrfs);
		if (entry >= 0 && entry < FAT_BLK_ENTRIES) {
			clust += entry;
			return clust;
		}
	}
	return FAT_CHAIN_END;
}

u_long rrfs_clustappend(fs_t fs, u_long clust)
{
	rrfs_t rrfs = (rrfs_t) fs->data;
	u_long newclust;
	int result;

	if (!RRFS_VALIDCLUST(rrfs, clust))
		return rrfs_clustalloc(fs);

	if ((newclust = rrfs_clustalloc(fs)) == FAT_CHAIN_END) {
#if _DEBUG
		kprintf("rrfs_clustappend: allocate cluster failed\n");
#endif
		return FAT_CHAIN_END;
	}
	if ((result = rrfs_readfatblk(fs, clust)) < 0) {
#if _DEBUG
		kprintf("rrfs_clustappend: read fatblk failed (%s)\n",
			strerror(result));
#endif
		return FAT_CHAIN_END;
	}
	rrfs->fatblk[clust % FAT_BLK_ENTRIES] = newclust;
	rrfs->flags |= RFS_FATBLK_DIRTY;
	return newclust;
}

#if _DEBUG
void rrfs_direntrydump(direntry_t de)
{
	int hour;

	kprintf("%c", (de->attr & DE_ATTR_DIR ? 'd' : '-'));
	kprintf("%c", (de->attr & DE_ATTR_READ ? 'r' : '-'));
	kprintf("%c", (de->attr & DE_ATTR_WRITE ? 'w' : '-'));
	kprintf("%c", (de->attr & DE_ATTR_EXEC ? 'x' : '-'));

	hour = (int)de->time[DE_TIME_HOUR];
	if (hour == 0)
		hour = 24;
	kprintf(" %2d", (hour > 12 ? hour - 12 : hour));

	kprintf(":%02d:%02d %2d-%02d-%04d",
		(int)de->time[DE_TIME_MIN],
		(int)de->time[DE_TIME_SEC],
		(int)de->date[DE_DATE_MON] + 1,
		(int)de->date[DE_DATE_DAY], (int)de->date[DE_DATE_YEAR] + 1900);

	kprintf("  %s\n", de->name);
}
#endif
