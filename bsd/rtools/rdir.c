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
#include <sys/types.h>
#include <unistd.h>
#include "rrfs.h"

void bufdump(char *buf, int size);
void printdirentry(direntry_t de);
void printmbr(mbr_t mbr);
uint32_t rrfs_nextclust(uint32_t clust, char *fat, uint32_t clusters);
int rrfs_readmbr(int devno, char *mbrbuf);
int rrfs_readfat(int devno, char *fat, int fatsectors);

int rrfs_find(int devno,
	      int clusters,
	      int fatsectors,
	      char *fat,
	      char *path,
	      int *filesize,
	      int *declust, int *deoff, int *firstclust, int *directory);

int main(int argc, char **argv)
{
	char *dir = NULL;
	char *dev = DEVICE;
	int devno;
	char mbrbuf[SECTOR_SIZE];
	mbr_t mbr;
	char *fat;
	char buf[SECTOR_SIZE];
	direntry_t de;
	uint32_t clust;
	int filesize, declust, deoff, firstclust, directory;
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
			dir = argv[i];
		}
	if (dir == NULL) {
		printf("missing directory name\n");
		exit(-1);
	}
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
	if (rrfs_readfat(devno, fat, mbr->params.fatsectors) < 0) {
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
		    BOOT_SECTORS + 2 * mbr->params.fatsectors + (int)clust;
		if (lseek(devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0) {
			printf("seek failed (%s)\n", strerror(errno));
			exit(-1);
		}
		if (read(devno, buf, SECTOR_SIZE) < 0) {
			printf("read failed (%s)\n", strerror(errno));
			exit(-1);
		}
		/* Display directory entries */
		for (de = (direntry_t) buf;; de++) {
			if (((u_char *) de->name)[0] == DE_UNUSED ||
			    ((char *)de - buf) >= SECTOR_SIZE)
				break;
			if (((u_char *) de->name)[0] != DE_DELETED) {
				bufdump((char *)de, DE_SIZE);
				printdirentry(de);
			}
		}

		clust = rrfs_nextclust(clust, fat, mbr->params.clusters);
		if (clust >= mbr->params.clusters)
			break;
	}
	close(devno);
	exit(0);
}
