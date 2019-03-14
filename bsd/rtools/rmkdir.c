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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "rrfs.h"

void getdir(char *path, char *dir);
void getname(char *path, char *name);
uint32_t rrfs_nextclust(uint32_t clust, char *fat, uint32_t clusters);
uint32_t rrfs_clustalloc(char *fat, uint32_t clusters);
int rrfs_readmbr(int devno, char *mbrbuf);
int rrfs_readfat(int devno, char *fat, int fatsectors);
int rrfs_writefat(int devno, char *fat, int fatsectors);

int rrfs_find(int devno,
	      int clusters,
	      int fatsectors,
	      char *fat,
	      char *path,
	      int *filesize,
	      int *declust, int *deoff, int *firstclust, int *directory);

void rrfs_mkdirentry(direntry_t de,
		     char *name, uint16_t attr, uint32_t size, uint32_t start);

int main(int argc, char **argv)
{
	char *dev = DEVICE, *path = NULL;
	char dir[PATH_LENGTH];
	char name[PATH_LENGTH];
	int devno;
	char mbrbuf[SECTOR_SIZE];
	mbr_t mbr;
	char *fat;
	char buf[SECTOR_SIZE];
	int filesize, declust, deoff, firstclust, directory;
	direntry_t de;
	uint32_t clust;
	int clustoff, i;

	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-d") == 0) {
			if (++i == argc) {
				printf("missing device name\n");
				exit(-1);
			}
			dev = argv[i];
		} else {
			if (argv[i][0] != '/') {
				printf("absolute path required\n");
				exit(-1);
			}
			path = argv[i];
		}
	if (path == NULL) {
		printf("missing file name\n");
		exit(-1);
	}
	bzero(dir, PATH_LENGTH);
	bzero(name, PATH_LENGTH);
	getdir(path, dir);
	getname(path, name);

	/* Device containing rrfs */
	if ((devno = open(dev, O_RDWR, 0)) < 0) {
		printf("could not open %s (%s)\n", dev, strerror(errno));
		exit(-1);
	}
	/* Read rrfs mbr */
	if (rrfs_readmbr(devno, mbrbuf) < 0) {
		printf("could not read mbr\n");
		exit(-1);
	}
	mbr = (mbr_t) mbrbuf;

	/* Read rrfs fat */
	fat = (char *)malloc((int)mbr->params.fatsectors * SECTOR_SIZE);
	bzero(fat, (int)mbr->params.fatsectors * SECTOR_SIZE);
	if (rrfs_readfat(devno, fat, (int)mbr->params.fatsectors) < 0) {
		printf("could not read fat\n");
		exit(-1);
	}
	/* Find the directory */
	if (rrfs_find(devno,
		      mbr->params.clusters,
		      mbr->params.fatsectors,
		      fat,
		      dir,
		      &filesize, &declust, &deoff, &firstclust,
		      &directory) < 0) {
		printf("%s not found\n", dir);
		exit(-1);
	}
	if (!directory) {
		printf("%s is not a directory\n", dir);
		exit(-1);
	}
	for (clust = (uint32_t) firstclust;;) {
		clustoff =
		    mbr->params.bootsectors + 2 * mbr->params.fatsectors +
		    (int)clust;
		if (lseek(devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0) {
			printf("seek failed (%s)\n", strerror(errno));
			exit(-1);
		}
		if (read(devno, buf, SECTOR_SIZE) < 0) {
			printf("read failed (%s)\n", strerror(errno));
			exit(-1);
		}
		/* Look for a free directory entry */
		for (de = (direntry_t) buf;;) {
			if (((u_char *) de->name)[0] == DE_UNUSED ||
			    ((u_char *) de->name)[0] == DE_DELETED) {
				uint32_t newclust;

				newclust =
				    rrfs_clustalloc(fat, mbr->params.clusters);
				if (newclust == FAT_CHAIN_END) {
					printf("file system full\n");
					exit(-1);
				}
				rrfs_mkdirentry(de,
						name,
						(uint16_t) DE_ATTR_DIR |
						DE_ATTR_READ |
						DE_ATTR_WRITE, (uint32_t) 0,
						(uint32_t) newclust);

				/* Save directory cluster */
				if (lseek
				    (devno, clustoff * SECTOR_SIZE,
				     SEEK_SET) < 0) {
					printf("seek failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				if (write(devno, buf, SECTOR_SIZE) < 0) {
					printf("write failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				/* Clear first cluster of new directory */
				clustoff =
				    mbr->params.bootsectors +
				    2 * mbr->params.fatsectors + (int)newclust;
				if (lseek
				    (devno, clustoff * SECTOR_SIZE,
				     SEEK_SET) < 0) {
					printf("seek failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				if (read(devno, buf, SECTOR_SIZE) < 0) {
					printf("read failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				bzero(buf, SECTOR_SIZE);

				if (lseek
				    (devno, clustoff * SECTOR_SIZE,
				     SEEK_SET) < 0) {
					printf("seek failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				if (write(devno, buf, SECTOR_SIZE) < 0) {
					printf("write failed (%s)\n",
					       strerror(errno));
					exit(-1);
				}
				/* Save the fat */
				if (rrfs_writefat
				    (devno, fat, mbr->params.fatsectors) < 0) {
					printf("could not write fat\n");
					exit(-1);
				}
				close(devno);
				exit(0);
			}
			if ((char *)(++de) - buf >= SECTOR_SIZE) {
				/* Finished with this cluster */
				clust =
				    rrfs_nextclust(clust, fat,
						   mbr->params.clusters);
				if (clust == FAT_CHAIN_END) {
					/* Need to add another cluster */
					printf("no more entries\n");
					exit(-1);
				}
			}
		}
	}
}
