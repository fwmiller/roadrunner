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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "rrfs.h"

void rrfs_mkdirentry(direntry_t de,
		     char *name, uint16_t attr, uint32_t size, uint32_t start);

static int
readfile(char *filename, char **filebuf, int *filesize)
{
    int fd, size;
    struct stat fdstat;

    if ((fd = open(filename, O_RDONLY)) < 0)
	return (-1);
    if (fstat(fd, &fdstat) < 0)
	return (-1);
#if _DEBUG
    printf("readfile: file size %d\n", (int) fdstat.st_size);
#endif
    *filesize = (int) fdstat.st_size;
    size = ((*filesize + SECTOR_SIZE - 1) / SECTOR_SIZE) * SECTOR_SIZE;
    if (*filebuf == NULL && (*filebuf = (char *) malloc(size)) == NULL)
	return (-1);
    if (read(fd, *filebuf, fdstat.st_size) < 0)
	return (-1);
    close(fd);
    return 0;
}

int
main(int argc, char **argv)
{
    char *dev = DEVICE;
    char *bootfile = NULL;
    char *bootfilebuf = NULL;
    char *fatbuf = NULL;
    uint32_t *fat;
    char rootdirbuf[SECTOR_SIZE];
    char *kernfile = NULL;
    char *kernfilebuf = NULL;
    mbr_t mbr;
    int bootfilesize, kernfilesize;
    int devno, i;

    /* 
     * These are the generated file system parameters.  The default values
     * are for a 3 1/2" 1.44 Mbyte floppy disk.
     */
    uint32_t tracks = 80;
    uint32_t heads = 2;
    uint32_t sectorspertrack = 18;
    uint16_t bytespersector = SECTOR_SIZE;
    uint16_t bootsectors = 16;
    uint32_t fatsectors = 24;
    uint16_t sectorsperclust = 1;
    uint32_t sectors;

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-d") == 0) {
	    if (++i == argc) {
		printf("missing device name\n");
		exit(-1);
	    }
	    dev = argv[i];
	} else if (strcmp(argv[i], "-b") == 0) {
	    if (++i == argc) {
		printf("missing boot file name\n");
		exit(-1);
	    }
	    bootfile = argv[i];
	} else if (strcmp(argv[i], "-k") == 0) {
	    if (++i == argc) {
		printf("missing kernel file name\n");
		exit(-1);
	    }
	    kernfile = argv[i];
	} else {
	    printf("illegal argument\n");
	    exit(-1);
	}
    }
#if _DEBUG
    if (bootfile != NULL)
	printf("boot file %s\n", bootfile);
#endif
    bootfilebuf = (char *) malloc(bootsectors * SECTOR_SIZE);
    if (bootfilebuf == NULL) {
	printf("malloc failed\n");
	exit(-1);
    }
    bzero(bootfilebuf, bootsectors * SECTOR_SIZE);

    /* Read boot file if specified */
    if (bootfile != NULL &&
	readfile(bootfile, &bootfilebuf, &bootfilesize) < 0) {
	printf("could not read boot file\n");
	exit(-1);
    }
#if _DEBUG
    if (kernfile != NULL)
	printf("kernel file %s\n", kernfile);
#endif
    /* Read kernel file if specified */
    if (kernfile != NULL &&
	readfile(kernfile, &kernfilebuf, &kernfilesize) < 0) {
	printf("could not read kernel file\n");
	exit(-1);
    }
    /* Setup file system parameters */
    sectors = tracks * heads * sectorspertrack;
    mbr = (mbr_t) bootfilebuf;
    mbr->params.tracks = tracks;
    mbr->params.heads = heads;
    mbr->params.sectorspertrack = sectorspertrack;
    mbr->params.bytespersector = bytespersector;
    mbr->params.sectors = sectors;
    mbr->params.bootsectors = bootsectors;
    mbr->params.fatsectors = fatsectors;
    mbr->params.sectorsperclust = sectorsperclust;
    mbr->params.clusters =
	(mbr->params.sectors -
	 (bootsectors +
	  2 * mbr->params.fatsectors)) / mbr->params.sectorsperclust;

    /* Initialize fat */
    fatbuf = (char *) malloc(mbr->params.fatsectors * SECTOR_SIZE);
    if (fatbuf == NULL) {
	printf("malloc failed\n");
	exit(-1);
    }
    bzero(fatbuf, mbr->params.fatsectors * SECTOR_SIZE);

    /* Add fat entry for the first root directory cluster */
    fat = (uint32_t *) fatbuf;
    fat[0] = FAT_CHAIN_END;

    /* Add fat entries for kernel if specified */
    if (kernfile != NULL) {
	uint32_t clustsize, clust, size;

	clustsize = mbr->params.sectorsperclust * SECTOR_SIZE;
	for (clust = 1, size = 0;
	     size < kernfilesize; clust++, size += clustsize)
	    if (size + clustsize >= kernfilesize)
		fat[clust] = FAT_CHAIN_END;
	    else
		fat[clust] = clust + 1;
    }
    /* Setup first root directory cluster */
    bzero(rootdirbuf, SECTOR_SIZE);

    /* Add root directory entry for kernel if specified */
    if (kernfile != NULL) {
	direntry_t de;

	de = (direntry_t) rootdirbuf;
	rrfs_mkdirentry(de,
			"kernel",
			(uint16_t) DE_ATTR_READ,
			(uint32_t) kernfilesize, (uint32_t) 1);
    }
    if ((devno = open(dev, O_RDWR)) < 0) {
	printf("could not open %s (%s)\n", dev, strerror(errno));
	exit(-1);
    }
    /* Write boot sectors */
    for (i = 0; i < bootsectors; i++) {
	if (lseek(devno, i * SECTOR_SIZE, SEEK_SET) < 0) {
	    printf("seek failed (%s)\n", strerror(errno));
	    exit(-1);
	}
	if (write(devno, bootfilebuf + i * SECTOR_SIZE, SECTOR_SIZE)
	    < 0) {
	    printf("write failed (%s)\n", strerror(errno));
	    exit(-1);
	}
    }
    /* Write fats */
    for (i = 0; i < mbr->params.fatsectors; i++) {
	if (lseek(devno, (bootsectors + i) * SECTOR_SIZE, SEEK_SET)
	    < 0) {
	    printf("seek failed (%s)\n", strerror(errno));
	    exit(-1);
	}
	if (write(devno, fatbuf + i * SECTOR_SIZE, SECTOR_SIZE) < 0) {
	    printf("write failed (%s)\n", strerror(errno));
	    exit(-1);
	}
    }
    for (i = 0; i < mbr->params.fatsectors; i++) {
	if (lseek(devno,
		  (bootsectors + mbr->params.fatsectors +
		   i) * SECTOR_SIZE, SEEK_SET) < 0) {
	    printf("seek failed (%s)\n", strerror(errno));
	    exit(-1);
	}
	if (write(devno, fatbuf + i * SECTOR_SIZE, SECTOR_SIZE) < 0) {
	    printf("write failed (%s)\n", strerror(errno));
	    exit(-1);
	}
    }
    /* Write first root directory cluster */
    if (lseek(devno,
	      (bootsectors + 2 * mbr->params.fatsectors) * SECTOR_SIZE,
	      SEEK_SET) < 0) {
	printf("seek failed (%s)\n", strerror(errno));
	exit(-1);
    }
    if (write(devno, rootdirbuf, SECTOR_SIZE) < 0) {
	printf("write failed (%s)\n", strerror(errno));
	exit(-1);
    }
    /* Write kernel if specified */
    if (kernfile != NULL) {
	int sector, size;

	for (sector =
	     bootsectors + 2 * mbr->params.fatsectors + 1, size =
	     0, i = 0; size < kernfilesize;
	     sector++, size += SECTOR_SIZE, i++) {
	    if (lseek(devno, sector * SECTOR_SIZE, SEEK_SET) < 0) {
		printf("seek failed (%s)\n", strerror(errno));
		exit(-1);
	    }
	    if (write(devno, kernfilebuf + i * SECTOR_SIZE, SECTOR_SIZE) <
		0) {
		printf("write failed (%s)\n", strerror(errno));
		exit(-1);
	    }
	}
    }
    close(devno);
    exit(0);
}
