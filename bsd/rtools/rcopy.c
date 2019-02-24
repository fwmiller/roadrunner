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
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "rrfs.h"

void getdir(char *path, char *dir);
void getname(char *path, char *name);
uint32_t rrfs_nextclust(uint32_t clust, char *fat, uint32_t clusters);
uint32_t rrfs_clustalloc(char *fat, uint32_t clusters);
uint32_t rrfs_clustappend(uint32_t clust, char *fat, uint32_t clusters);
int rrfs_readmbr(int devno, char *mbrbuf);
int rrfs_readfat(int devno, char *fat, int fatsectors);
int rrfs_writefat(int devno, char *fat, int fatsectors);

int rrfs_find(int devno,
	      int clusters,
	      int fatsectors,
	      char *fat,
	      char *path,
	      int *filesize,
	      int *declust,
	      int *deoff,
	      int *firstclust,
	      int *directory);

void rrfs_mkdirentry(direntry_t de,
		     char *name, uint16_t attr, uint32_t size, uint32_t start);

int
main(int argc, char **argv)
{
    char *dev = DEVICE;
    struct stat fdstat;
    char iname[PATH_LENGTH];
    char dir[PATH_LENGTH];
    char name[PATH_LENGTH];
    int fd, devno;
    char mbrbuf[SECTOR_SIZE];
    mbr_t mbr;
    char *fat, *dst;
    char buf[SECTOR_SIZE];
    uint32_t clustoff, clust;
    int firstclust, dstfirstclust, dirfirstclust;
    int filesize, declust, deoff, directory, len;
    int dstfilesize, dirfilesize, i;
    direntry_t de;

    for (i = 1; i < argc; i++)
	if (strcmp(argv[i], "-d") == 0) {
	    if (++i == argc) {
		printf("missing device name\n");
		exit(-1);
	    }
	    dev = argv[i];
	}
    if (argv[argc - 1][0] != '/') {
	printf("absolute path required\n");
	exit(-1);
    }
    /* Device containing rrfs */
    if ((devno = open(dev, O_RDWR)) < 0) {
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
    fat = (char *) malloc((int) mbr->params.fatsectors * SECTOR_SIZE);
    bzero(fat, (int) mbr->params.fatsectors * SECTOR_SIZE);
    if (rrfs_readfat(devno, fat, (int) mbr->params.fatsectors) < 0) {
	printf("could not read fat\n");
	exit(-1);
    }
    for (i = 1; i < argc - 1; i++) {

	/* Skip device argument */
	if (strcmp(argv[i], "-d") == 0) {
	    i++;
	    continue;
	}
	/* Input file */
	if ((fd = open(argv[i], O_RDONLY, 0)) < 0) {
	    printf("could not open %s (%s)\n", argv[i], strerror(errno));
	    continue;
	}
	if (fstat(fd, &fdstat) < 0) {
	    printf("stat failed (%s)\n", strerror(errno));
	    continue;
	}
	if (!(fdstat.st_mode & S_IFREG)) {
	    printf("%s is not a regular file\n", argv[i]);
	    continue;
	}
	bzero(iname, PATH_LENGTH);
	bzero(dir, PATH_LENGTH);
	bzero(name, PATH_LENGTH);
	getname(argv[i], iname);
	getdir(argv[argc - 1], dir);
	getname(argv[argc - 1], name);

	/* Copy file */
	for (firstclust = 0, filesize = 0;;) {
	    /* Read a cluster's worth from the file */
	    if ((len = read(fd, buf, SECTOR_SIZE)) < 0) {
		printf("read failed (%s)\n", strerror(errno));
		goto nextfile;
	    }
	    if (len == 0)
		break;
	    filesize += len;

	    if (firstclust == 0)
		firstclust = clust =
		    rrfs_clustalloc(fat, mbr->params.clusters);
	    else
		clust = rrfs_clustappend(clust, fat, mbr->params.clusters);

	    clustoff = mbr->params.bootsectors +
		2 * mbr->params.fatsectors + (int) clust;
	    if (lseek(devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0) {
		printf("seek failed (%s)\n", strerror(errno));
		goto nextfile;
	    }
	    if (write(devno, buf, SECTOR_SIZE) < 0) {
		printf("write failed (%s)\n", strerror(errno));
		goto nextfile;
	    }
	}
	/* 
	 * First check whether the specified destination is a directory or a
	 * file name.  Heres how this is done.  First try to find the
	 * specified destination.  If it is not found, assume it was
	 * specified as a new file name and look for the enclosing directory.
	 * If it is found, check whether it is a directory.  If it is, put
	 * the file there using the input file name.  If not, fail since
	 * rcopy does not overwrite existing files.
	 */
	if (rrfs_find(devno,
		      mbr->params.clusters,
		      mbr->params.fatsectors,
		      fat,
		      argv[argc - 1],
		      &dstfilesize,
		      &declust, &deoff, &dstfirstclust, &directory) < 0)
	    /* Destination not found so look for the enclosing directory */
	    dst = dir;
	else if (directory) {
	    /* Destination is directory */
	    dst = argv[argc - 1];
	    strcpy(name, iname);

	    /* XXX Need to check whether this file name exists */

	} else {
	    printf("could not overwrite %s\n", argv[argc - 1]);
	    continue;
	}
#if _DEBUG
	printf("copy %s to directory %s name %s\n", argv[i], dst, name);
#endif
	/* Destination not found so look for the enclosing directory */
	if (rrfs_find(devno,
		      mbr->params.clusters,
		      mbr->params.fatsectors,
		      fat,
		      dst,
		      &dirfilesize,
		      &declust, &deoff, &dirfirstclust, &directory) < 0) {
	    printf("%s not found\n", argv[argc - 1]);
	    continue;
	}
	if (!directory) {
	    printf("%s is not a directory\n", argv[argc - 1]);
	    continue;
	}
	for (clust = dirfirstclust;;) {
	    clustoff = mbr->params.bootsectors +
		2 * mbr->params.fatsectors + (int) clust;
	    if (lseek(devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0) {
		printf("seek failed (%s)\n", strerror(errno));
		goto nextfile;
	    }
	    if (read(devno, buf, SECTOR_SIZE) < 0) {
		printf("read failed (%s)\n", strerror(errno));
		goto nextfile;
	    }
	    /* Look for free directory entry */
	    for (de = (direntry_t) buf;;) {
		if (((u_char *) de->name)[0] == DE_UNUSED ||
		    ((u_char *) de->name)[0] == DE_DELETED) {
		    rrfs_mkdirentry(de, name, (uint16_t)
				    DE_ATTR_READ |
				    DE_ATTR_WRITE,
				    (uint32_t) filesize,
				    (uint32_t) firstclust);

		    /* Save directory cluster */
		    clustoff = mbr->params.bootsectors +
			2 * mbr->params.fatsectors + (int) clust;
		    if (lseek(devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0)
			printf("seek failed (%s)\n", strerror(errno));
		    if (write(devno, buf, SECTOR_SIZE) < 0)
			printf("write failed (%s)\n", strerror(errno));
		    goto nextfile;
		}
		if ((char *) (++de) - buf >= SECTOR_SIZE) {
		    uint32_t newclust;

		    /* Finished with this cluster */
		    newclust =
			rrfs_nextclust(clust, fat, mbr->params.clusters);
		    if (newclust == FAT_CHAIN_END) {
			/* Need to add another cluster */
			newclust =
			    rrfs_clustappend(clust,
					     fat, mbr->params.clusters);
			if (newclust == FAT_CHAIN_END) {
			    printf("file system full\n");
			    exit(-1);
			}
			bzero(buf, SECTOR_SIZE);
		    } else {
			/* Read next directory cluster */
			clustoff =
			    mbr->params.bootsectors +
			    2 * mbr->params.fatsectors + (int) newclust;
			if (lseek
			    (devno, clustoff * SECTOR_SIZE, SEEK_SET) < 0) {
			    printf("seek failed (%s)\n", strerror(errno));
			    goto nextfile;
			}
			if (read(devno, buf, SECTOR_SIZE) < 0) {
			    printf("read failed (%s)\n", strerror(errno));
			    goto nextfile;
			}
		    }
		    clust = newclust;
		    de = (direntry_t) buf;
		}
	    }
	}
      nextfile:;
    }
    /* Save the fat */
    if (rrfs_writefat(devno, fat, mbr->params.fatsectors) < 0) {
	printf("could not write fat\n");
	exit(-1);
    }
    close(devno);
    exit(0);
}
