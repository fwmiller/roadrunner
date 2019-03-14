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

#include <dev.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fs/rrfs.h>
#include <sys.h>
#include <sys/ioctl.h>
#include <sys/part.h>
#include <unistd.h>

static int read_bootfile(char *bootfile, char **bootbuf)
{
	u_long filesize;
	int fd, result;
	ssize_t len;

	*bootbuf = (char *)malloc(RRFS_BOOT_SECTORS * SECTOR_SIZE);
	if (*bootbuf == NULL)
		return ENOMEM;
	bzero(*bootbuf, RRFS_BOOT_SECTORS * SECTOR_SIZE);

	if ((result = open(bootfile, O_RDONLY)) < 0)
		goto bootfile_openerror;
	fd = result;

	if ((result = ioctl(fd, GET_FILE_SIZE, &filesize)) < 0)
		goto bootfile_error;

	if (filesize >= RRFS_BOOT_SECTORS * SECTOR_SIZE) {
		result = EFBIG;
		goto bootfile_error;
	}
	if ((len = read(fd, *bootbuf, (size_t) filesize)) < 0) {
		result = (int)len;
		goto bootfile_error;
	}
	close(fd);
	return 0;

 bootfile_error:
	close(fd);
 bootfile_openerror:
	free(*bootbuf);
	*bootbuf = NULL;
	return result;
}

static int write_sector(int devno, u_long blkno, char *buf)
{
	struct seek seekargs;
	int result;

	seekargs.offset = blkno;
	seekargs.whence = SEEK_SET;
	result = ioctl(devno, SEEK_BLOCK, &seekargs);
	if (result < 0) {
		printf("write_sector: seek failed (%s)\n", strerror(result));
		return result;
	}
	result = write(devno, buf, SECTOR_SIZE);
	if (result < 0) {
		printf("write_sector: write failed (%s)\n", strerror(result));
		return result;
	}
	return 0;
}

static void
format_mbr(mbr_t mbr, geometry_t geom, u_long size, int *fatsectors,
	   int *sectorsperclust)
{
	u_long clusters;

	mbr->params.tracks = geom->tracks;
	mbr->params.heads = geom->heads;
	mbr->params.sectorspertrack = geom->sectorspertrack;
	mbr->params.bytespersector = SECTOR_SIZE;
	mbr->params.sectors =
	    mbr->params.tracks * mbr->params.heads *
	    mbr->params.sectorspertrack;

	if (mbr->params.sectors <= 2880)
		/* Assume a SECTOR_SIZE byte cluster for floppy disks */
		mbr->params.sectorsperclust = 1;
	else
		/* Assume a PAGE_SIZE byte cluster for hard disks */
		mbr->params.sectorsperclust = PAGE_SIZE / SECTOR_SIZE;
	*sectorsperclust = (int)mbr->params.sectorsperclust;

	/* Must be a multiple of mbr->params.sectorsperclust */
	mbr->params.bootsectors = RRFS_BOOT_SECTORS;

	/* Total possible clusters */
	clusters = size / mbr->params.sectorsperclust;

	/* Fat sectors */
	mbr->params.fatsectors =
	    ALIGN(clusters / (SECTOR_SIZE / sizeof(u_long)),
		  mbr->params.sectorsperclust);
	*fatsectors = (int)mbr->params.fatsectors;

	/* Actual data clusters */
	mbr->params.clusters =
	    clusters -
	    ((mbr->params.bootsectors + 2 * mbr->params.fatsectors) /
	     mbr->params.sectorsperclust);

#if _DEBUG
	printf("format_mbr: %u sec/clust %u boot sec %u fat sec %u clust\n",
	       (u_int) mbr->params.sectorsperclust,
	       (u_int) mbr->params.bootsectors,
	       (u_int) mbr->params.fatsectors, (u_int) mbr->params.clusters);
#endif
}

int main(int argc, char **argv)
{
	char *bootfile = NULL;
	int writembr = 0;
	char *device = NULL;
	char *bootbuf = NULL;
	struct geometry geom;
	char buf[SECTOR_SIZE];
	u_long offset;
	int fatsectors, sectorsperclust;
	int devno, i, partsize, result;

	/* Command line arguments */
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-b") == 0) {
			if (++i == argc) {
				printf("missing boot file name\n");
				return EINVAL;
			}
			bootfile = argv[i];
		} else if (strcmp(argv[i], "-mbr") == 0)
			writembr = 1;
		else if (device == NULL)
			device = argv[i];
		else {
			printf("illegal parameter\n");
			return EINVAL;
		}
	if (device == NULL || strncmp(device, "/dev", 4) != 0) {
		printf("device expected\n");
		return EINVAL;
	}
#if _DEBUG
	printf("format: device %s\n", device);
#endif

	/* Read boot file if specified */
	if (bootfile != NULL) {
		result = read_bootfile(bootfile, &bootbuf);
		if (result < 0)
			return result;
	}
	/* Open disk device */
	result = open(device, O_RDWR);
	if (result < 0) {
		printf("could not open %s (%s)\n", device, strerror(result));
		goto open_error;
	}
	devno = result;

	/* Lock disk device */
	result = ioctl(devno, LOCK, NULL);
	if (result < 0) {
		printf("lock failed (%s)\n", strerror(result));
		goto lock_error;
	}
	/* Get disk geometry */
	result = ioctl(devno, GET_GEOMETRY, &geom);
	if (result < 0) {
		printf("get geometry failed (%s)\n", strerror(result));
		goto format_error;
	}
	/* Get partition size */
	result = ioctl(devno, GET_PART_SIZE, &partsize);
	if (result < 0) {
		printf("get partition offset failed (%s)\n", strerror(result));
		goto format_error;
	}
#if _DEBUG
	printf("format: partition size %u sectors\n", partsize);
#endif
	offset = 0;

    /*************************************************************************
    * Format mbr                                                             *
    *************************************************************************/

	/* Read existing mbr */
	if (writembr && bootbuf != NULL) {
		char mbrbuf[SECTOR_SIZE];

		result = ioctl(devno, READ_MBR, mbrbuf);
		if (result < 0) {
			printf("read mbr failed (%s)\n", strerror(result));
			goto format_error;
		}
		/* 
		 * A boot program was specified.  The first sector of the boot
		 * program has the partition table copied into it from the
		 * existing mbr and is written as the new mbr.
		 */
		bcopy(mbrbuf + PART_ENT_1,
		      bootbuf + PART_ENT_1, PARTS * PART_ENT_SIZE);

		result = ioctl(devno, WRITE_MBR, bootbuf);
		if (result < 0) {
			printf("write mbr failed (%s)\n", strerror(result));
			goto format_error;
		}
	}
	if (bootbuf == NULL) {
		/* 
		 * No boot program was specified.  Put the rrfs format
		 * information into a sector buffer and write it to the first
		 * sector of the specified partition.
		 */
		format_mbr((mbr_t) buf, &geom, partsize,
			   &fatsectors, &sectorsperclust);

		result = write_sector(devno, offset, buf);
		if (result < 0) {
			printf("write first partition sector failed (%s)\n",
			       strerror(result));
			goto format_error;
		}

	} else {
		/* 
		 * A boot program was specified.  Put the rrfs format
		 * information into the first sector of the boot program code
		 * and write the entired boot program to the beginning of the
		 * specified partition.
		 */
		format_mbr((mbr_t) bootbuf, &geom, partsize,
			   &fatsectors, &sectorsperclust);

		for (i = 0; i < RRFS_BOOT_SECTORS; i++) {
			result = write_sector(devno, offset + i,
					      bootbuf + i * SECTOR_SIZE);
			if (result < 0) {
				printf("write boot program sector ");
				printf("%d failed (%s)\n", i, strerror(result));
				goto format_error;
			}
		}
	}

    /*************************************************************************
    * Format fat                                                             *
    *************************************************************************/

	/* Format the first fat sector */
	bzero(buf, SECTOR_SIZE);
	*((u_long *) buf) = (u_long) FAT_CHAIN_END;

	/* Write the first fat sector */
	offset += RRFS_BOOT_SECTORS;
	result = write_sector(devno, offset, buf);
	if (result < 0) {
		printf("write first fat sector failed (%s)\n",
		       strerror(result));
		goto format_error;
	}
	result = write_sector(devno, offset + fatsectors, buf);
	if (result < 0) {
		printf("write first fat sector failed (%s)\n",
		       strerror(result));
		goto format_error;
	}
	/* Format for the remaining fat sectors */
	bzero(buf, SECTOR_SIZE);

	/* Write the remaining fat sectors */
	for (i = 1; i < fatsectors; i++) {
		result = write_sector(devno, offset + i, buf);
		if (result < 0) {
			printf("write fat sector %d failed (%s)\n",
			       i + 1, strerror(result));
			goto format_error;
		}
		result = write_sector(devno, offset + fatsectors + i, buf);
		if (result < 0) {
			printf("write fat sector %d failed (%s)\n",
			       i + 1, strerror(result));
			goto format_error;
		}
	}

    /*************************************************************************
    * Format first root directory cluster                                    *
    *************************************************************************/

	/* Format for the sectors of the first root directory cluster */
	bzero(buf, SECTOR_SIZE);

	/* Write the sectors of the first root directory cluster */
	offset += 2 * fatsectors;
	for (i = 0; i < sectorsperclust; i++) {
		result = write_sector(devno, offset + i, buf);
		if (result < 0) {
			printf("write sector %d ", i);
			printf
			    ("of first root directory cluster failed (%s)\n",
			     strerror(result));
			goto format_error;
		}
	}

	result = 0;
 format_error:
	ioctl(devno, UNLOCK, NULL);
 lock_error:
	close(devno);
 open_error:
	return result;
}
