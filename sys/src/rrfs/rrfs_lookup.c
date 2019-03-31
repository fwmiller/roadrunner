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
#include <sys/buf.h>
#include <sys/mem.h>

int rrfs_lookup(file_t file, char *path)
{
	rrfs_t rrfs;
	rrfile_t rrfile;
	buf_t b;
	char *lastpath, *currpath;
	char name[DE_NAME_LENGTH];
	u_long clust, declust, deoff, filesize, firstclust;
	int pos, result;

	rrfs = (rrfs_t) file->fs->data;
	rrfile = (rrfile_t) file->data;

	/* Special case for root directory */
	if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
		file->flags |= F_DIR;
		file->filesize = 0;
		rrfile->flags |= RF_ROOTDIR;
		rrfile->declust = 0;
		rrfile->deoff = 0;
		rrfile->firstclust = 0;
		rrfile->currclust = 0;
		return 0;
	}
	/* Set these up for path traversal */
	if ((lastpath = (char *)malloc(PATH_LENGTH)) == NULL)
		return ENOMEM;
	if ((currpath = (char *)malloc(PATH_LENGTH)) == NULL) {
		free(lastpath);
		return ENOMEM;
	}
	bzero(lastpath, PATH_LENGTH);
	bzero(currpath, PATH_LENGTH);
	strcpy(currpath, "/");

#if 0
	if ((result = flock(rfs.fsno, currpath)) < 0)
		goto flockerror;
#endif

	/* Search the file system hierarchy for the specified full path */
	for (firstclust = 0, filesize = 0, pos = 0;;) {
		int found = 0;

		/* Get the next element from full path */
		rrfs_nextpathelem(path, &pos, name);

		/* Add element to current path */
		strcpy(lastpath, currpath);
		if (strcmp(currpath, "/") != 0)
			strcat(currpath, "/");
		strcat(currpath, name);

#if 0
		/* 
		 * As the file system directories are traversed, each will
		 * be flocked in turn.  A technique called `crabbing' from
		 * database systems is used to make sure things work right.
		 * As we move to a new directory, the lock for the old
		 * directory is held until the lock for the new directory
		 * is aquired.
		 */
		if ((result = flock(rfs.fsno, currpath)) < 0) {
			frelease(lastpath);
			goto flockerror;
		}
		frelease(lastpath);
#endif

		/* Search a directory for a path element */
		for (clust = firstclust, deoff = 0;;) {
			direntry_t de;
			int i;

			b = bget(file->bufsize);
			blen(b) = file->bufsize;
			if ((result = rrfs_readclust(file, clust, &b)) < 0) {
#if _DEBUG
				kprintf
				    ("rrfs_lookup: read cluster failed (%s)\n",
				     strerror(result));
#endif
				goto readclusterror;
			}
			/* 
			 * Search a directory cluster.  The search will find
			 * a matching entry or exhaust the entries in the
			 * cluster.  If a matching entry is found, it can be
			 * a directory or a file.  If it is a directory, it
			 * might be the end of the search or just one step
			 * along the full path.  If it is not a directory,
			 * it must be the end of the search.
			 */
			for (i = 0; i < file->bufsize; i += DE_SIZE) {
				de = (direntry_t) (bstart(b) + i);

				if ((u_char) de->name[0] == DE_UNUSED) {
					result = ENOENT;
					goto filenotfound;
				}
				if (strcmp(de->name, name) == 0) {
					/* Found matching directory entry */
					filesize = de->size;
					declust = firstclust;
					deoff += i;
					firstclust = de->start;
					found = 1;

					if (path[pos] == '/') {
						/* 
						 * There are more elements on
						 * the specified full path to
						 * consider so this entry must
						 * be a directory.
						 */
						if (!(de->attr & DE_ATTR_DIR)) {
#if _DEBUG
							kprintf
							    ("rrfs_lookup: ");
							kprintf
							    ("directory expected\n");
#endif
							result = ENOENT;
							goto filenotfound;
						}
					} else {
						/* 
						 * Reached the end of the
						 * specified full path with
						 * a matching entry.  Set a
						 * flag if the entry is for
						 * a directory.
						 */
						if (de->attr & DE_ATTR_DIR)
							file->flags |= F_DIR;
						goto filefound;
					}
					/* 
					 * Found a directory somewhere before
					 * the end of the specified full path
					 * so continue the search with the
					 * new directory.
					 */
					break;
				}
			}
			brel(b);
			b = NULL;

			if (found)
				break;

			deoff +=
			    rrfs->mbr->params.sectorsperclust * SECTOR_SIZE;

			/* Get the next cluster number in the directory */
			clust = rrfs_nextclust(file->fs, clust);
			if (clust >= rrfs->mbr->params.clusters) {
				/* File not found */
				result = ENOENT;
				goto filenotfound;
			}
		}
	}
 filefound:
	file->filesize = filesize;
	rrfile->declust = declust;
	rrfile->deoff = deoff;
	rrfile->firstclust = firstclust;
	rrfile->currclust = firstclust;

 filenotfound:
 readclusterror:
	if (b != NULL)
		brel(b);
#if 0
	frelease(currpath);
 flockerror:
#endif
	free(lastpath);
	free(currpath);
	return result;
}
