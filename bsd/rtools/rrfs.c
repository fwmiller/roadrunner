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
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rrfs.h"

#define VALIDCLUST(C) ((C) < clusters)

void printmbr(mbr_t mbr);

/* 
 * Find the next cluster number in a fat chain.  Returns the next cluster
 * number if it is found and the value FAT_CHAIN_END otherwise.
 */
uint32_t rrfs_nextclust(uint32_t clust, char *fat, uint32_t clusters)
{
	uint32_t c;

	if (VALIDCLUST(clust)) {
		c = ((uint32_t *) fat)[clust];
		if (VALIDCLUST(c))
			return c;
		return FAT_CHAIN_END;
	}
	return FAT_CHAIN_END;
}

/* 
 * Allocate a cluster in a fat.  Returns the number of the allocated cluster
 * if one is found and the value FAT_CHAIN_END otherwise.
 */
uint32_t rrfs_clustalloc(char *fat, uint32_t clusters)
{
	uint32_t c;

	/* 
	 * XXX Use a simple linear search beginning from the start of the
	 * partition.  This won't be adequate for very long.
	 */
	for (c = 0; c < clusters; c++)
		if (((uint32_t *) fat)[c] == 0) {
			((uint32_t *) fat)[c] = FAT_CHAIN_END;
			return c;
		}
	return FAT_CHAIN_END;
}

/* 
 * Append a cluster to a fat chain. Returns the cluster number added to the
 * chain if one is found and the value FAT_CHAIN_END otherwise.
 */
uint32_t rrfs_clustappend(uint32_t clust, char *fat, uint32_t clusters)
{
	if (VALIDCLUST(clust)) {
		uint32_t c;

		if ((c = rrfs_clustalloc(fat, clusters)) == FAT_CHAIN_END)
			return FAT_CHAIN_END;
		((uint32_t *) fat)[clust] = c;
		return c;
	} else if (clust == FAT_CHAIN_END)
		return rrfs_clustalloc(fat, clusters);

	return FAT_CHAIN_END;
}

int rrfs_readmbr(int devno, char *mbrbuf)
{
	mbr_t mbr;

	if (lseek(devno, 0, SEEK_SET) < 0) {
#if _DEBUG
		printf("rrfs_readmbr: seek failed (%s)\n", strerror(errno));
#endif
		return (-1);
	}
	if (read(devno, mbrbuf, SECTOR_SIZE) < 0) {
#if _DEBUG
		printf("rrfs_readmbr: read failed (%s)\n", strerror(errno));
#endif
		return (-1);
	}
	mbr = (mbr_t) mbrbuf;
	if (mbr->params.bytespersector != SECTOR_SIZE) {
#if _DEBUG
		printf("rrfs_readmbr: not an rrfs file system\n");
#endif
		return (-1);
	}
#if _DEBUG
	printmbr(mbr);
#endif
	return 0;
}

int rrfs_writembr(int devno, char *mbrbuf)
{
	mbr_t mbr = (mbr_t) mbrbuf;

	if (mbr->params.bytespersector != 9) {
#if _DEBUG
		printf("rrfs_writembr: not an rrfs file system\n");
#endif
		return (-1);
	}
	if (lseek(devno, 0, SEEK_SET) < 0) {
#if _DEBUG
		printf("rrfs_writembr: seek failed (%s)\n", strerror(errno));
#endif
		return (-1);
	}
	if (write(devno, mbrbuf, SECTOR_SIZE) < 0) {
#if _DEBUG
		printf("rrfs_writembr: write failed (%s)\n", strerror(errno));
#endif
		return (-1);
	}
	return 0;
}

int rrfs_readfat(int devno, char *fat, int fatsectors)
{
	int i;

	if (lseek(devno, 0, SEEK_SET) < 0) {
#if _DEBUG
		printf("rrfs_readfat: initial seek failed (%s)\n",
		       strerror(errno));
#endif
		return (-1);
	}
	for (i = 0; i < fatsectors; i++) {
		if (lseek(devno, (BOOT_SECTORS + i) * SECTOR_SIZE, SEEK_SET)
		    < 0) {
#if _DEBUG
			printf("rrfs_readfat: seek failed (%s)\n",
			       strerror(errno));
#endif
			return (-1);
		}
		if (read(devno, fat + (i * SECTOR_SIZE), SECTOR_SIZE) < 0) {
#if _DEBUG
			printf("rrfs_readfat: read failed (%s)\n",
			       strerror(errno));
#endif
			return (-1);
		}
	}
	return 0;
}

int rrfs_writefat(int devno, char *fat, int fatsectors)
{
	int i;

	if (lseek(devno, 0, SEEK_SET) < 0) {
#if _DEBUG
		printf("rrfs_writefat: initial seek failed (%s)\n",
		       strerror(errno));
#endif
		return (-1);
	}
	for (i = 0; i < fatsectors; i++) {
		if (lseek(devno, (BOOT_SECTORS + i) * SECTOR_SIZE, SEEK_SET)
		    < 0) {
#if _DEBUG
			printf("rrfs_writefat: seek failed (%s)\n",
			       strerror(errno));
#endif
			return (-1);
		}
		if (write(devno, fat + (i * SECTOR_SIZE), SECTOR_SIZE) < 0) {
#if _DEBUG
			printf("rrfs_writefat: write failed (%s)\n",
			       strerror(errno));
#endif
			return (-1);
		}
	}
	return 0;
}
