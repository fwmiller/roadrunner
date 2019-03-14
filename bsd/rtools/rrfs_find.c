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
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rrfs.h"

#define FILENAMECHAR(C)							\
    (isalnum(C) || (C) == '_' || (C) == '.' || (C) == '-' ||		\
     (C) == '%' || (C) == '(' || (C) == ')' || (C) == '+' ||		\
     (C) == ',' || (C) == '$' || (C) == '#' || (C) == ':' ||		\
     (C) == ';' || (C) == '<' || (C) == '=' || (C) == '>' ||		\
     (C) == '?' || (C) == '@' || (C) == '[' || (C) == ']' ||		\
     (C) == '^' || (C) == '!' || (C) == '{' || (C) == '|' ||		\
     (C) == '}' || (C) == '~')

uint32_t rrfs_nextclust(uint32_t clust, char *fat, uint32_t clusters);

static void nextpathelem(char *path, int *pos, char *name)
{
	int i;

	bzero(name, DE_NAME_LENGTH);
	if (path[*pos] == '/')
		(*pos)++;
	for (i = 0;
	     i < DE_NAME_LENGTH - 1 && FILENAMECHAR(path[*pos]); i++, (*pos)++)
		name[i] = path[*pos];
}

#if _DEBUG
static void
printfstat(int filesize, int declust, int deoff, int firstclust, int directory)
{
	if (directory)
		printf("directory ");
	printf("size %d declust %d deoff %d firstclust %d\n",
	       filesize, declust, deoff, firstclust);
}
#endif

/* 
 * This routine will traverse the directories in a path looking for
 * information on the file at the end of the path.
 */
int
rrfs_find(int devno,
	  int clusters,
	  int fatsectors,
	  char *fat,
	  char *path,
	  int *filesize,
	  int *declust, int *deoff, int *firstclust, int *directory)
{
	char name[PATH_LENGTH];
	char buf[SECTOR_SIZE];
	int clust, pos;

	/* Special case for root directory */
	if (strcmp(path, "/") == 0) {
		*filesize = 0;
		*declust = 0;
		*deoff = 0;
		*firstclust = 0;
		*directory = 1;
#if _DEBUG
		printfstat(*filesize, *declust, *deoff, *firstclust,
			   *directory);
#endif
		return 0;
	}
	/* Initial seek */
	if (lseek(devno, 0, SEEK_SET) < 0) {
#if _DEBUG
		printf("rrfs_find: initial seek failed (%s)\n",
		       strerror(errno));
#endif
		return (-1);
	}
	bzero(buf, SECTOR_SIZE);

	for (*directory = 0, *firstclust = 0, *filesize = 0, pos = 0;;) {
		int found = 0;

		/* Get next path element */
		nextpathelem(path, &pos, name);

		for (clust = *firstclust;;) {
			direntry_t de;
			int offset, i;

			/* Read next cluster */
			offset = BOOT_SECTORS + 2 * fatsectors + clust;
			offset *= SECTOR_SIZE;
			if (lseek(devno, offset, SEEK_SET) < 0) {
#if _DEBUG
				printf("rrfs_find: seek failed (%s)\n",
				       strerror(errno));
#endif
				return (-1);
			}
			if (read(devno, buf, SECTOR_SIZE) < 0) {
#if _DEBUG
				printf("rrfs_find: read failed (%s)\n",
				       strerror(errno));
#endif
				return (-1);
			}
			/* Search cluster for path element name */
			for (i = 0; i < SECTOR_SIZE; i += DE_SIZE) {
				de = (direntry_t) (buf + i);

				if (((u_char *) de->name)[0] == DE_UNUSED) {
#if _DEBUG
					printf("rrfs_find: %s not found\n",
					       name);
#endif
					return (-1);
				}
				if (strcmp(de->name, name) == 0) {
					/* Found matching directory entry */
					*filesize = de->size;
					*declust = *firstclust;
					*deoff = i;
					*firstclust = de->start;
					found = 1;

					if (path[pos] == '/') {
						/* 
						 * There are more elements on the specified full path
						 * to consider so this entry must be a directory.
						 */
						if (!(de->attr & DE_ATTR_DIR)) {
#if _DEBUG
							printf
							    ("rrfs_find: directory expected\n");
#endif
							return (-1);
						}
					} else {
						/* 
						 * Reached the end of the specified full path with a
						 * matching entry.  Set a flag if the entry is for a
						 * directory.
						 */
						if (de->attr & DE_ATTR_DIR)
							*directory = 1;
#if _DEBUG
						printfstat(*filesize,
							   *declust, *deoff,
							   *firstclust,
							   *directory);
#endif
						return 0;
					}
					/* 
					 * Found a directory somewhere before the end of the
					 * specified full path so continue the search with the
					 * new directory.
					 */
					break;
				}
			}
			if (found)
				break;

			/* Get the next cluster number in the directory */
			if ((clust = rrfs_nextclust(clust, fat, clusters))
			    >= clusters) {
#if _DEBUG
				printf("rrfs_find: not found\n");
#endif
				return (-1);
			}
		}
	}
	return (-1);
}
